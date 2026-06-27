/*
 * fused.c — the FUSE "kernel" engine. See fused.h.
 *
 * Implements the kernel side of the FUSE protocol (fuse_kernel.h) so a guest
 * libfuse daemon thinks it is talking to a real kernel FUSE driver, while the
 * proot redirect drives it with a path-based VFS API to serve guest syscalls
 * under the mountpoint.
 *
 * Model: synchronous, one outstanding request at a time. Each op is
 *   build fuse_in_header + body  ->  write(chan)  ->  read(chan) reply  ->
 *   translate fuse_out_header + body. The channel is one end of the daemon's
 *   /dev/fuse SOCK_SEQPACKET pair, so one datagram == one FUSE message.
 *
 * Node IDs: the daemon addresses inodes by 64-bit nodeid (root == 1, FUSE_ROOT_ID).
 * The redirect speaks in paths, so we keep a path->nodeid cache and resolve a
 * path by walking it component-by-component with FUSE_LOOKUP from the nearest
 * known ancestor, caching each step. Sufficient and correct for the read/write
 * VFS ops; FORGET is deferred to fused_destroy (we hold one lookup ref per
 * cached node, released in bulk — libfuse tolerates this).
 */
#include "fused.h"
#include "fuse_kernel.h"

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdint.h>

/* Single FUSE message ceiling: 128 KiB payload + headroom for headers. Bounds
 * one read/write/readdir round-trip; callers loop for more. */
#define FUSED_MAXDATA  (128 * 1024)
#define FUSED_MSGCAP   (FUSED_MAXDATA + 4096)

struct fnode {
	char    *path;       /* mount-relative, e.g. "/", "/a/b" */
	uint64_t nodeid;
};

struct fused {
	int       fd;             /* channel (our end of /dev/fuse) */
	uint32_t  uid, gid, pid;
	uint64_t  unique;
	uint32_t  proto_minor;
	uint32_t  max_write;      /* negotiated FUSE_INIT max_write */
	int       inited;
	/* path -> nodeid cache (root is implicit) */
	struct fnode *nodes;
	size_t        nnodes, capnodes;
	int  (*io_send)(void *ctx, const void *buf, size_t len);   /* transport (proot), or NULL */
	int  (*io_recv)(void *ctx, void *buf, size_t cap, uint64_t unique);
	void  *io_ctx;
};

void fused_set_io(fused_t *f,
                  int (*send)(void *ctx, const void *buf, size_t len),
                  int (*recv)(void *ctx, void *buf, size_t cap, uint64_t unique),
                  void *ctx)
{
	if (!f) return;
	f->io_send = send;
	f->io_recv = recv;
	f->io_ctx = ctx;
}

/* Send one request message (header+body, already in buf). 0 on success, -errno. */
static int tx_send(fused_t *f, const void *buf, size_t len)
{
	if (f->io_send) { int r = f->io_send(f->io_ctx, buf, len); return r < 0 ? r : 0; }
	for (;;) {
		ssize_t w = write(f->fd, buf, len);
		if (w < 0) { if (errno == EINTR) continue; return -errno; }
		if ((size_t) w != len) return -EIO;
		return 0;
	}
}

/* Receive the reply for `unique` into buf (<= cap). Returns bytes, or -errno.
 * The io transport (proot) multiplexes by unique so concurrent ops from several
 * processes on one mount don't cross replies; the fd path (host test) is
 * sequential and ignores `unique`. */
static ssize_t tx_recv(fused_t *f, void *buf, size_t cap, uint64_t unique)
{
	if (f->io_recv) return f->io_recv(f->io_ctx, buf, cap, unique);
	for (;;) {
		ssize_t r = read(f->fd, buf, cap);
		if (r < 0) { if (errno == EINTR) continue; return -errno; }
		return r;
	}
}

/* ---- low-level request/reply ----
 * RE-ENTRANT: each call uses its own heap message buffer (no shared f->msg), so
 * an op suspended mid-round-trip (while the proot redirect pumps the event loop
 * and another process issues a FUSE op on the SAME engine) doesn't clobber it. */

/* Send (opcode,nodeid,in[inlen]); copy up to outcap reply-body bytes into out,
 * report the full body length in *outlen. 0, or negative errno. */
static int xfer(fused_t *f, uint32_t opcode, uint64_t nodeid,
                const void *in, size_t inlen,
                void *out, size_t outcap, size_t *outlen)
{
	if (outlen) *outlen = 0;
	size_t total = sizeof(struct fuse_in_header) + inlen;
	if (total > FUSED_MSGCAP) return -EINVAL;
	unsigned char *m = malloc(FUSED_MSGCAP);
	if (!m) return -ENOMEM;

	struct fuse_in_header h;
	memset(&h, 0, sizeof h);
	h.len = (uint32_t) total;
	h.opcode = opcode;
	h.unique = ++f->unique;
	h.nodeid = nodeid;
	h.uid = f->uid; h.gid = f->gid; h.pid = f->pid;
	memcpy(m, &h, sizeof h);
	if (inlen) memcpy(m + sizeof h, in, inlen);

	int sr = tx_send(f, m, total);
	if (sr < 0) { free(m); return sr; }
	ssize_t r = tx_recv(f, m, FUSED_MSGCAP, h.unique);
	if (r < 0) { free(m); return (int) r; }
	if ((size_t) r < sizeof(struct fuse_out_header)) { free(m); return -EIO; }

	struct fuse_out_header oh;
	memcpy(&oh, m, sizeof oh);
	if (oh.error) { int e = oh.error < 0 ? oh.error : -oh.error; free(m); return e; }
	size_t body = (size_t) r - sizeof oh;
	if (out && body) { size_t n = body < outcap ? body : outcap; memcpy(out, m + sizeof oh, n); }
	if (outlen) *outlen = body;
	free(m);
	return 0;
}

/* xfer alias for variable-size replies (READ/READDIR/READLINK) — same as xfer;
 * the caller supplies the output buffer (no returned pointer into shared state). */
#define xfer_var xfer

/* Fire-and-forget request (replyless: FUSE_FORGET; teardown: FUSE_DESTROY). */
static void xfer_noreply(fused_t *f, uint32_t opcode, uint64_t nodeid,
                         const void *in, size_t inlen)
{
	size_t total = sizeof(struct fuse_in_header) + inlen;
	if (total > FUSED_MSGCAP) return;
	unsigned char *m = malloc(total);
	if (!m) return;
	struct fuse_in_header h;
	memset(&h, 0, sizeof h);
	h.len = (uint32_t) total;
	h.opcode = opcode;
	h.unique = ++f->unique;
	h.nodeid = nodeid;
	h.uid = f->uid; h.gid = f->gid; h.pid = f->pid;
	memcpy(m, &h, sizeof h);
	if (inlen) memcpy(m + sizeof h, in, inlen);
	(void) tx_send(f, m, total);
	free(m);
}

/* ---- node cache ---- */


static void cache_put(fused_t *f, const char *path, uint64_t nodeid)
{
	for (size_t i = 0; i < f->nnodes; i++)
		if (strcmp(f->nodes[i].path, path) == 0) { f->nodes[i].nodeid = nodeid; return; }
	if (f->nnodes == f->capnodes) {
		size_t nc = f->capnodes ? f->capnodes * 2 : 16;
		struct fnode *nn = realloc(f->nodes, nc * sizeof *nn);
		if (!nn) return;            /* cache is best-effort; drop on OOM */
		f->nodes = nn; f->capnodes = nc;
	}
	f->nodes[f->nnodes].path = strdup(path);
	if (!f->nodes[f->nnodes].path) return;
	f->nodes[f->nnodes].nodeid = nodeid;
	f->nnodes++;
}

static void cache_drop(fused_t *f, const char *path)
{
	for (size_t i = 0; i < f->nnodes; i++)
		if (strcmp(f->nodes[i].path, path) == 0) {
			free(f->nodes[i].path);
			f->nodes[i] = f->nodes[--f->nnodes];
			return;
		}
}

/* Canonicalize a mount-relative path: collapse "" / "." segments and resolve
 * ".." lexically (like the kernel — a libfuse daemon never sees a LOOKUP for "."
 * or "..", so we must never send one). Result is absolute, no trailing slash
 * ("/" for the root). */
static void normpath(const char *in, char *out, size_t cap)
{
	const char *seg[256];
	int segn[256];
	int n = 0;
	const char *p = in;
	while (*p) {
		while (*p == '/') p++;
		if (!*p) break;
		const char *s = p;
		while (*p && *p != '/') p++;
		int len = (int)(p - s);
		if (len == 1 && s[0] == '.') continue;
		if (len == 2 && s[0] == '.' && s[1] == '.') { if (n > 0) n--; continue; }
		if (n < 256) { seg[n] = s; segn[n] = len; n++; }
	}
	if (n == 0) { snprintf(out, cap, "/"); return; }
	size_t o = 0;
	for (int i = 0; i < n && o + 1 < cap; i++) {
		out[o++] = '/';
		int len = segn[i];
		for (int k = 0; k < len && o + 1 < cap; k++) out[o++] = seg[i][k];
	}
	out[o] = '\0';
}

/* dir/base split: "/a/b" -> dir "/a", base "b"; "/a" -> dir "/", base "a". */
static void split_path(const char *path, char *dir, size_t dcap, char *base, size_t bcap)
{
	const char *slash = strrchr(path, '/');
	if (!slash) { snprintf(dir, dcap, "/"); snprintf(base, bcap, "%s", path); return; }
	snprintf(base, bcap, "%s", slash + 1);
	if (slash == path) snprintf(dir, dcap, "/");
	else { size_t n = (size_t)(slash - path); if (n >= dcap) n = dcap - 1; memcpy(dir, path, n); dir[n] = '\0'; }
}

static void attr_to_stat(const struct fuse_attr *a, struct stat *st)
{
	memset(st, 0, sizeof *st);
	st->st_ino     = a->ino;
	st->st_mode    = a->mode;
	st->st_nlink   = a->nlink;
	st->st_uid     = a->uid;
	st->st_gid     = a->gid;
	st->st_rdev    = a->rdev;
	st->st_size    = (off_t) a->size;
	st->st_blksize = a->blksize ? a->blksize : 4096;
	st->st_blocks  = (blkcnt_t) a->blocks;
	st->st_atime   = (time_t) a->atime;
	st->st_mtime   = (time_t) a->mtime;
	st->st_ctime   = (time_t) a->ctime;
}

/* One FUSE_LOOKUP of `name` in `parent`; fills entry_out. 0 or -errno. */
static int do_lookup(fused_t *f, uint64_t parent, const char *name, struct fuse_entry_out *eo)
{
	size_t ol;
	int rc = xfer(f, FUSE_LOOKUP, parent, name, strlen(name) + 1, eo, sizeof *eo, &ol);
	if (rc) return rc;
	if (ol < sizeof *eo) return -EIO;
	return 0;
}

/* Resolve a mount-relative path to a nodeid, FOLLOWING symlinks like the kernel:
 * the kernel FUSE driver resolves symlinks (READLINK + restart) before sending
 * OPEN/GETATTR, so a libfuse daemon never gets an OPEN on a symlink node (it
 * errors). follow_final controls whether the LAST component is dereferenced
 * (1 for getattr/open/read..., 0 for readlink/lstat). 0/-errno. */
static int resolve_x(fused_t *f, const char *rawpath, uint64_t *out, int follow_final)
{
	char path[4096];
	normpath(rawpath, path, sizeof path);
	int loops = 0;
restart:
	if (++loops > 40) return -ELOOP;
	if (path[0] == '/' && path[1] == '\0') { *out = FUSE_ROOT_ID; return 0; }

	uint64_t cur = FUSE_ROOT_ID;
	char curdir[4096]; curdir[0] = '/'; curdir[1] = '\0';   /* dir containing `cur`'s child */
	const char *p = path + 1;                               /* skip leading '/' */
	while (*p) {
		char comp[256]; int ci = 0;
		while (*p && *p != '/' && ci < (int) sizeof comp - 1) comp[ci++] = *p++;
		comp[ci] = '\0';
		int is_last = (*p == '\0');
		while (*p == '/') p++;

		struct fuse_entry_out eo;
		int rc = do_lookup(f, cur, comp, &eo);
		if (rc) return rc;

		/* cache the resolved child path -> nodeid (for FORGET at destroy) */
		char child[4096];
		if (curdir[1] == '\0') snprintf(child, sizeof child, "/%s", comp);
		else snprintf(child, sizeof child, "%s/%s", curdir, comp);
		cache_put(f, child, eo.nodeid);

		if ((eo.attr.mode & 0170000) == 0120000 /*S_IFLNK*/ && (!is_last || follow_final)) {
			char tgt[4096]; size_t bl;
			rc = xfer_var(f, FUSE_READLINK, eo.nodeid, NULL, 0, tgt, sizeof tgt - 1, &bl);
			if (rc) return rc;
			size_t tl = bl < sizeof tgt - 1 ? bl : sizeof tgt - 1;
			tgt[tl] = '\0';
			char newp[8192];
			if (tgt[0] == '/') snprintf(newp, sizeof newp, "%s/%s", tgt, p);
			else snprintf(newp, sizeof newp, "%s/%s/%s", curdir, tgt, p);
			normpath(newp, path, sizeof path);
			goto restart;
		}
		cur = eo.nodeid;
		snprintf(curdir, sizeof curdir, "%s", child);
	}
	*out = cur;
	return 0;
}

static int resolve(fused_t *f, const char *path, uint64_t *out)
{ return resolve_x(f, path, out, 1); }

/* Resolve a path's PARENT nodeid + give back the basename (for create/unlink). */
static int resolve_parent(fused_t *f, const char *path, uint64_t *parent, char *base, size_t bcap)
{
	char dir[4096];
	split_path(path, dir, sizeof dir, base, bcap);
	return resolve(f, dir, parent);   /* parent dir is followed (symlinked dirs ok) */
}

/* ---- lifecycle ---- */

fused_t *fused_new(int chan_fd, uint32_t uid, uint32_t gid)
{
	fused_t *f = calloc(1, sizeof *f);
	if (!f) return NULL;
	f->fd = chan_fd;
	f->uid = uid; f->gid = gid;
	f->pid = (uint32_t) getpid();
	f->unique = 0;
	f->max_write = FUSED_MAXDATA;
	return f;
}

int fused_init(fused_t *f)
{
	struct fuse_init_in in;
	memset(&in, 0, sizeof in);
	in.major = FUSE_KERNEL_VERSION;
	in.minor = FUSE_KERNEL_MINOR_VERSION;
	in.max_readahead = FUSED_MAXDATA;
	in.flags = 0;            /* decline splice/writeback/etc.: keep the wire minimal */

	struct fuse_init_out out;
	size_t ol;
	int rc = xfer(f, FUSE_INIT, 0, &in, sizeof in, &out, sizeof out, &ol);
	if (rc) return rc;
	if (ol < 8) return -EPROTO;        /* need at least major/minor */
	if (out.major != FUSE_KERNEL_VERSION) return -EPROTO;
	f->proto_minor = out.minor;
	if (ol >= offsetof(struct fuse_init_out, time_gran) && out.max_write)
		f->max_write = out.max_write < FUSED_MAXDATA ? out.max_write : FUSED_MAXDATA;
	f->inited = 1;
	return 0;
}

void fused_destroy(fused_t *f)
{
	if (!f) return;
	if (f->inited) {
		/* Release lookup refs, then DESTROY — both are reply-less here: FORGET
		 * never gets a reply by protocol, and we don't wait for DESTROY's (we
		 * close the channel right after, so the daemon's loop ends on EOF). */
		for (size_t i = 0; i < f->nnodes; i++) {
			struct fuse_forget_in fi = { .nlookup = 1 };
			xfer_noreply(f, FUSE_FORGET, f->nodes[i].nodeid, &fi, sizeof fi);
		}
		xfer_noreply(f, FUSE_DESTROY, 0, NULL, 0);
	}
	for (size_t i = 0; i < f->nnodes; i++) free(f->nodes[i].path);
	free(f->nodes);
	free(f);
}

/* ---- read-side ops ---- */

int fused_getattr(fused_t *f, const char *path, struct stat *st)
{
	uint64_t nid;
	int rc = resolve(f, path, &nid);
	if (rc) return rc;
	struct fuse_getattr_in in;
	memset(&in, 0, sizeof in);
	struct fuse_attr_out out;
	size_t ol;
	rc = xfer(f, FUSE_GETATTR, nid, &in, sizeof in, &out, sizeof out, &ol);
	if (rc) return rc;
	if (ol < sizeof out) return -EIO;
	attr_to_stat(&out.attr, st);
	return 0;
}

int fused_readlink(fused_t *f, const char *path, char *buf, size_t cap)
{
	uint64_t nid;
	int rc = resolve_x(f, path, &nid, 0);   /* don't deref the final symlink */
	if (rc) return rc;
	size_t bl;
	rc = xfer_var(f, FUSE_READLINK, nid, NULL, 0, buf, cap - 1, &bl);
	if (rc) return rc;
	if (bl >= cap) bl = cap - 1;
	buf[bl] = '\0';
	return 0;
}

int fused_access(fused_t *f, const char *path, int mask)
{
	uint64_t nid;
	int rc = resolve(f, path, &nid);
	if (rc) return rc;
	struct fuse_access_in in;
	memset(&in, 0, sizeof in);
	in.mask = (uint32_t) mask;
	rc = xfer(f, FUSE_ACCESS, nid, &in, sizeof in, NULL, 0, NULL);
	if (rc == -ENOSYS) return 0;   /* daemon declines ACCESS -> allow */
	return rc;
}

int fused_statfs(fused_t *f, const char *path, struct statvfs *out)
{
	uint64_t nid;
	if (resolve(f, path, &nid) != 0) nid = FUSE_ROOT_ID;
	struct fuse_statfs_out so;
	size_t ol;
	int rc = xfer(f, FUSE_STATFS, nid, NULL, 0, &so, sizeof so, &ol);
	if (rc) return rc;
	if (ol < sizeof so) return -EIO;
	memset(out, 0, sizeof *out);
	out->f_bsize   = so.st.bsize;
	out->f_frsize  = so.st.frsize ? so.st.frsize : so.st.bsize;
	out->f_blocks  = so.st.blocks;
	out->f_bfree   = so.st.bfree;
	out->f_bavail  = so.st.bavail;
	out->f_files   = so.st.files;
	out->f_ffree   = so.st.ffree;
	out->f_favail  = so.st.ffree;
	out->f_namemax = so.st.namelen;
	return 0;
}

int fused_open(fused_t *f, const char *path, int flags, uint64_t *fh)
{
	uint64_t nid;
	int rc = resolve(f, path, &nid);
	if (rc) return rc;
	struct fuse_open_in in;
	memset(&in, 0, sizeof in);
	in.flags = (uint32_t) flags;
	struct fuse_open_out out;
	size_t ol;
	rc = xfer(f, FUSE_OPEN, nid, &in, sizeof in, &out, sizeof out, &ol);
	if (rc) return rc;
	if (ol < sizeof out) return -EIO;
	if (fh) *fh = out.fh;
	return 0;
}

int fused_read(fused_t *f, const char *path, uint64_t fh, void *buf, size_t size, off_t off)
{
	uint64_t nid;
	int rc = resolve(f, path, &nid);
	if (rc) return rc;
	if (size > f->max_write) size = f->max_write;   /* one round-trip cap */
	struct fuse_read_in in;
	memset(&in, 0, sizeof in);
	in.fh = fh;
	in.offset = (uint64_t) off;
	in.size = (uint32_t) size;
	size_t bl;
	rc = xfer_var(f, FUSE_READ, nid, &in, sizeof in, buf, size, &bl);
	if (rc) return rc;
	if (bl > size) bl = size;
	return (int) bl;
}

int fused_write(fused_t *f, const char *path, uint64_t fh, const void *buf, size_t size, off_t off)
{
	uint64_t nid;
	int rc = resolve(f, path, &nid);
	if (rc) return rc;
	if (size > f->max_write) size = f->max_write;
	if (sizeof(struct fuse_in_header) + sizeof(struct fuse_write_in) + size > FUSED_MSGCAP)
		size = FUSED_MSGCAP - sizeof(struct fuse_in_header) - sizeof(struct fuse_write_in);
	struct fuse_write_in wi;
	memset(&wi, 0, sizeof wi);
	wi.fh = fh;
	wi.offset = (uint64_t) off;
	wi.size = (uint32_t) size;

	/* one packet: header + fuse_write_in + data (own buffer, re-entrant) */
	unsigned char *m = malloc(FUSED_MSGCAP);
	if (!m) return -ENOMEM;
	struct fuse_in_header h;
	memset(&h, 0, sizeof h);
	h.len = (uint32_t)(sizeof h + sizeof wi + size);
	h.opcode = FUSE_WRITE;
	h.unique = ++f->unique;
	h.nodeid = nid;
	h.uid = f->uid; h.gid = f->gid; h.pid = f->pid;
	memcpy(m, &h, sizeof h);
	memcpy(m + sizeof h, &wi, sizeof wi);
	memcpy(m + sizeof h + sizeof wi, buf, size);
	int sr = tx_send(f, m, h.len);
	if (sr < 0) { free(m); return sr; }
	ssize_t r = tx_recv(f, m, FUSED_MSGCAP, h.unique);
	if (r < 0) { free(m); return (int) r; }
	if ((size_t) r < sizeof(struct fuse_out_header)) { free(m); return -EIO; }
	struct fuse_out_header oh; memcpy(&oh, m, sizeof oh);
	if (oh.error) { int e = oh.error < 0 ? oh.error : -oh.error; free(m); return e; }
	struct fuse_write_out wo;
	if ((size_t) r - sizeof oh < sizeof wo) { free(m); return -EIO; }
	memcpy(&wo, m + sizeof oh, sizeof wo);
	free(m);
	return (int) wo.size;
}

int fused_flush(fused_t *f, const char *path, uint64_t fh)
{
	uint64_t nid;
	int rc = resolve(f, path, &nid);
	if (rc) return rc;
	struct fuse_flush_in in;
	memset(&in, 0, sizeof in);
	in.fh = fh;
	rc = xfer(f, FUSE_FLUSH, nid, &in, sizeof in, NULL, 0, NULL);
	if (rc == -ENOSYS) return 0;
	return rc;
}

int fused_fsync(fused_t *f, const char *path, uint64_t fh, int datasync)
{
	uint64_t nid;
	int rc = resolve(f, path, &nid);
	if (rc) return rc;
	struct fuse_fsync_in in;
	memset(&in, 0, sizeof in);
	in.fh = fh;
	in.fsync_flags = datasync ? 1 : 0;
	rc = xfer(f, FUSE_FSYNC, nid, &in, sizeof in, NULL, 0, NULL);
	if (rc == -ENOSYS) return 0;
	return rc;
}

int fused_release(fused_t *f, const char *path, uint64_t fh)
{
	uint64_t nid;
	int rc = resolve(f, path, &nid);
	if (rc) return rc;
	struct fuse_release_in in;
	memset(&in, 0, sizeof in);
	in.fh = fh;
	rc = xfer(f, FUSE_RELEASE, nid, &in, sizeof in, NULL, 0, NULL);
	if (rc == -ENOSYS) return 0;
	return rc;
}

int fused_pread(fused_t *f, const char *path, void *buf, size_t size, off_t off)
{
	uint64_t fh = 0;
	int rc = fused_open(f, path, O_RDONLY, &fh);
	if (rc) return rc;
	int n = fused_read(f, path, fh, buf, size, off);
	fused_release(f, path, fh);
	return n;
}

int fused_pwrite(fused_t *f, const char *path, const void *buf, size_t size, off_t off)
{
	uint64_t fh = 0;
	int rc = fused_open(f, path, O_WRONLY, &fh);
	if (rc) return rc;
	int n = fused_write(f, path, fh, buf, size, off);
	fused_flush(f, path, fh);
	fused_release(f, path, fh);
	return n;
}

int fused_readdir(fused_t *f, const char *path,
                  void (*emit)(void *ctx, const char *name, unsigned type), void *ctx)
{
	uint64_t nid;
	int rc = resolve(f, path, &nid);
	if (rc) return rc;

	/* OPENDIR */
	struct fuse_open_in oin;
	memset(&oin, 0, sizeof oin);
	struct fuse_open_out oout;
	size_t ol;
	rc = xfer(f, FUSE_OPENDIR, nid, &oin, sizeof oin, &oout, sizeof oout, &ol);
	uint64_t dh = 0;
	if (rc == 0 && ol >= sizeof oout) dh = oout.fh;
	else if (rc != -ENOSYS && rc != 0) return rc;   /* ENOSYS: stateless readdir */

	uint64_t off = 0;
	int ret = 0;
	for (;;) {
		struct fuse_read_in rin;
		memset(&rin, 0, sizeof rin);
		rin.fh = dh;
		rin.offset = off;
		rin.size = 4096;
		unsigned char dbuf[4096]; const unsigned char *body = dbuf; size_t bl;
		rc = xfer_var(f, FUSE_READDIR, nid, &rin, sizeof rin, dbuf, sizeof dbuf, &bl);
		if (rc) { ret = rc; break; }
		if (bl > sizeof dbuf) bl = sizeof dbuf;
		if (bl == 0) break;     /* end of stream */

		size_t p = 0;
		int progressed = 0;
		while (p + sizeof(struct fuse_dirent) <= bl) {
			struct fuse_dirent de;
			memcpy(&de, body + p, sizeof de);
			size_t namelen = de.namelen;
			size_t rec = FUSE_DIRENT_ALIGN(sizeof(struct fuse_dirent) + namelen);
			if (namelen == 0 || p + sizeof(struct fuse_dirent) + namelen > bl) break;
			char nm[256];
			size_t cn = namelen < sizeof nm - 1 ? namelen : sizeof nm - 1;
			memcpy(nm, body + p + sizeof(struct fuse_dirent), cn);
			nm[cn] = '\0';
			if (emit) emit(ctx, nm, de.type);
			off = de.off;       /* next read resumes here */
			p += rec;
			progressed = 1;
		}
		if (!progressed) break;
	}

	if (dh) {
		struct fuse_release_in ri;
		memset(&ri, 0, sizeof ri);
		ri.fh = dh;
		(void) xfer(f, FUSE_RELEASEDIR, nid, &ri, sizeof ri, NULL, 0, NULL);
	}
	return ret;
}

/* ---- write-side ops ---- */

int fused_create(fused_t *f, const char *path, int flags, mode_t mode, uint64_t *fh)
{
	uint64_t parent; char base[256];
	int rc = resolve_parent(f, path, &parent, base, sizeof base);
	if (rc) return rc;

	size_t blen = strlen(base) + 1;
	unsigned char in[sizeof(struct fuse_create_in) + 256];
	struct fuse_create_in ci;
	memset(&ci, 0, sizeof ci);
	ci.flags = (uint32_t) flags;
	ci.mode  = (uint32_t) mode;
	ci.umask = 0;
	memcpy(in, &ci, sizeof ci);
	memcpy(in + sizeof ci, base, blen);

	/* reply = fuse_entry_out + fuse_open_out */
	unsigned char out[sizeof(struct fuse_entry_out) + sizeof(struct fuse_open_out)];
	size_t ol;
	rc = xfer(f, FUSE_CREATE, parent, in, sizeof ci + blen, out, sizeof out, &ol);
	if (rc) return rc;
	if (ol < sizeof(struct fuse_entry_out) + sizeof(struct fuse_open_out)) return -EIO;
	struct fuse_entry_out eo; memcpy(&eo, out, sizeof eo);
	struct fuse_open_out oo; memcpy(&oo, out + sizeof eo, sizeof oo);
	cache_put(f, path, eo.nodeid);
	if (fh) *fh = oo.fh;
	return 0;
}

int fused_mkdir(fused_t *f, const char *path, mode_t mode)
{
	uint64_t parent; char base[256];
	int rc = resolve_parent(f, path, &parent, base, sizeof base);
	if (rc) return rc;
	size_t blen = strlen(base) + 1;
	unsigned char in[sizeof(struct fuse_mkdir_in) + 256];
	struct fuse_mkdir_in mi;
	memset(&mi, 0, sizeof mi);
	mi.mode = (uint32_t) mode;
	mi.umask = 0;
	memcpy(in, &mi, sizeof mi);
	memcpy(in + sizeof mi, base, blen);
	struct fuse_entry_out eo;
	size_t ol;
	rc = xfer(f, FUSE_MKDIR, parent, in, sizeof mi + blen, &eo, sizeof eo, &ol);
	if (rc) return rc;
	if (ol >= sizeof eo) cache_put(f, path, eo.nodeid);
	return 0;
}

static int name_op(fused_t *f, uint32_t opcode, const char *path)
{
	uint64_t parent; char base[256];
	int rc = resolve_parent(f, path, &parent, base, sizeof base);
	if (rc) return rc;
	rc = xfer(f, opcode, parent, base, strlen(base) + 1, NULL, 0, NULL);
	if (rc == 0) cache_drop(f, path);
	return rc;
}

int fused_unlink(fused_t *f, const char *path) { return name_op(f, FUSE_UNLINK, path); }
int fused_rmdir (fused_t *f, const char *path) { return name_op(f, FUSE_RMDIR,  path); }

int fused_rename(fused_t *f, const char *from, const char *to)
{
	uint64_t oldp, newp; char ob[256], nb[256];
	int rc = resolve_parent(f, from, &oldp, ob, sizeof ob);
	if (rc) return rc;
	rc = resolve_parent(f, to, &newp, nb, sizeof nb);
	if (rc) return rc;
	size_t ol = strlen(ob) + 1, nl = strlen(nb) + 1;
	unsigned char in[sizeof(struct fuse_rename_in) + 512];
	if (sizeof(struct fuse_rename_in) + ol + nl > sizeof in) return -ENAMETOOLONG;
	struct fuse_rename_in ri;
	memset(&ri, 0, sizeof ri);
	ri.newdir = newp;
	memcpy(in, &ri, sizeof ri);
	memcpy(in + sizeof ri, ob, ol);
	memcpy(in + sizeof ri + ol, nb, nl);
	rc = xfer(f, FUSE_RENAME, oldp, in, sizeof ri + ol + nl, NULL, 0, NULL);
	if (rc == 0) { cache_drop(f, from); cache_drop(f, to); }
	return rc;
}

int fused_symlink(fused_t *f, const char *target, const char *linkpath)
{
	uint64_t parent; char base[256];
	int rc = resolve_parent(f, linkpath, &parent, base, sizeof base);
	if (rc) return rc;
	size_t bl = strlen(base) + 1, tl = strlen(target) + 1;
	unsigned char in[512 + 4096];
	if (bl + tl > sizeof in) return -ENAMETOOLONG;
	memcpy(in, base, bl);
	memcpy(in + bl, target, tl);
	struct fuse_entry_out eo;
	size_t ol;
	rc = xfer(f, FUSE_SYMLINK, parent, in, bl + tl, &eo, sizeof eo, &ol);
	if (rc == 0 && ol >= sizeof eo) cache_put(f, linkpath, eo.nodeid);
	return rc;
}

/* SETATTR helper */
static int setattr(fused_t *f, const char *path, uint32_t valid, const struct fuse_setattr_in *src)
{
	uint64_t nid;
	int rc = resolve(f, path, &nid);
	if (rc) return rc;
	struct fuse_setattr_in in = *src;
	in.valid = valid;
	struct fuse_attr_out out;
	size_t ol;
	return xfer(f, FUSE_SETATTR, nid, &in, sizeof in, &out, sizeof out, &ol);
}

int fused_truncate(fused_t *f, const char *path, off_t size)
{
	struct fuse_setattr_in in; memset(&in, 0, sizeof in);
	in.size = (uint64_t) size;
	return setattr(f, path, FATTR_SIZE, &in);
}

int fused_chmod(fused_t *f, const char *path, mode_t mode)
{
	struct fuse_setattr_in in; memset(&in, 0, sizeof in);
	in.mode = (uint32_t) mode;
	return setattr(f, path, FATTR_MODE, &in);
}

int fused_chown(fused_t *f, const char *path, uid_t uid, gid_t gid)
{
	struct fuse_setattr_in in; memset(&in, 0, sizeof in);
	uint32_t valid = 0;
	if (uid != (uid_t) -1) { in.uid = uid; valid |= FATTR_UID; }
	if (gid != (gid_t) -1) { in.gid = gid; valid |= FATTR_GID; }
	if (!valid) return 0;
	return setattr(f, path, valid, &in);
}

int fused_utimens(fused_t *f, const char *path, const struct timespec tv[2])
{
	struct fuse_setattr_in in; memset(&in, 0, sizeof in);
	in.atime = (uint64_t) tv[0].tv_sec; in.atimensec = (uint32_t) tv[0].tv_nsec;
	in.mtime = (uint64_t) tv[1].tv_sec; in.mtimensec = (uint32_t) tv[1].tv_nsec;
	return setattr(f, path, FATTR_ATIME | FATTR_MTIME, &in);
}

/*
 * fused_hello_test.c — validate the `fused` FUSE-kernel engine against a REAL
 * libfuse3 daemon, the same library sshfs/rclone/gocryptfs/ntfs-3g use.
 *
 * A child process runs a tiny in-memory read/write filesystem via the libfuse3
 * HIGH-LEVEL API, but instead of mounting (which needs root/kernel FUSE) it
 * drives the session over one end of a SOCK_SEQPACKET socketpair via
 * fuse_session_custom_io(). The parent runs `fused` on the other end and
 * exercises the VFS API: INIT, getattr, readdir, open/read, create/write,
 * mkdir, unlink, truncate. If fused's wire format matches what libfuse expects,
 * every op round-trips correctly.
 *
 * Build: cc fused_hello_test.c fused.c -lfuse3   (needs libfuse3-dev)
 * Exit 0 iff all checks pass.
 */
#define FUSE_USE_VERSION 31
#include <fuse3/fuse.h>
#include <fuse3/fuse_lowlevel.h>   /* struct fuse_custom_io, fuse_session_custom_io */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "fused.h"

/* ───────────────────── in-memory fs (the libfuse daemon) ───────────────────── */

struct mnode { char path[128]; int isdir; mode_t mode; char *data; size_t size; int used; };
#define MAXN 64
static struct mnode g_n[MAXN];

static struct mnode *mfind(const char *p) {
	for (int i = 0; i < MAXN; i++) if (g_n[i].used && strcmp(g_n[i].path, p) == 0) return &g_n[i];
	return NULL;
}
static struct mnode *mnew(const char *p, int isdir, mode_t mode) {
	for (int i = 0; i < MAXN; i++) if (!g_n[i].used) {
		struct mnode *n = &g_n[i];
		memset(n, 0, sizeof *n);
		snprintf(n->path, sizeof n->path, "%s", p);
		n->isdir = isdir; n->mode = mode; n->used = 1;
		return n;
	}
	return NULL;
}

static int mm_getattr(const char *path, struct stat *st, struct fuse_file_info *fi) {
	(void) fi;
	struct mnode *n = mfind(path);
	if (!n) return -ENOENT;
	memset(st, 0, sizeof *st);
	if (n->isdir) { st->st_mode = S_IFDIR | (n->mode ? n->mode : 0755); st->st_nlink = 2; }
	else { st->st_mode = S_IFREG | (n->mode ? n->mode : 0644); st->st_nlink = 1; st->st_size = n->size; }
	return 0;
}
static int mm_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t off,
                      struct fuse_file_info *fi, enum fuse_readdir_flags flags) {
	(void) off; (void) fi; (void) flags;
	if (!mfind(path)) return -ENOENT;
	filler(buf, ".", NULL, 0, 0);
	filler(buf, "..", NULL, 0, 0);
	size_t plen = strlen(path);
	for (int i = 0; i < MAXN; i++) {
		if (!g_n[i].used) continue;
		const char *p = g_n[i].path;
		if (strcmp(p, "/") == 0) continue;
		/* direct children of `path` only */
		const char *rest;
		if (strcmp(path, "/") == 0) rest = p + 1;
		else { if (strncmp(p, path, plen) != 0 || p[plen] != '/') continue; rest = p + plen + 1; }
		if (*rest == '\0' || strchr(rest, '/')) continue;
		filler(buf, rest, NULL, 0, 0);
	}
	return 0;
}
static int mm_open(const char *path, struct fuse_file_info *fi) {
	(void) fi; return mfind(path) ? 0 : -ENOENT;
}
static int mm_read(const char *path, char *buf, size_t size, off_t off, struct fuse_file_info *fi) {
	(void) fi;
	struct mnode *n = mfind(path);
	if (!n || n->isdir) return -ENOENT;
	if ((size_t) off >= n->size) return 0;
	size_t avail = n->size - off;
	if (size > avail) size = avail;
	memcpy(buf, n->data + off, size);
	return (int) size;
}
static int mm_write(const char *path, const char *buf, size_t size, off_t off, struct fuse_file_info *fi) {
	(void) fi;
	struct mnode *n = mfind(path);
	if (!n || n->isdir) return -ENOENT;
	if (off + size > n->size) {
		char *d = realloc(n->data, off + size);
		if (!d) return -ENOMEM;
		if ((size_t) off > n->size) memset(d + n->size, 0, off - n->size);
		n->data = d; n->size = off + size;
	}
	memcpy(n->data + off, buf, size);
	return (int) size;
}
static int mm_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
	(void) fi;
	if (mfind(path)) return -EEXIST;
	return mnew(path, 0, mode) ? 0 : -ENOSPC;
}
static int mm_mkdir(const char *path, mode_t mode) {
	if (mfind(path)) return -EEXIST;
	return mnew(path, 1, mode) ? 0 : -ENOSPC;
}
static int mm_unlink(const char *path) {
	struct mnode *n = mfind(path);
	if (!n) return -ENOENT;
	free(n->data); n->used = 0; return 0;
}
static int mm_rmdir(const char *path) { return mm_unlink(path); }
static int mm_truncate(const char *path, off_t size, struct fuse_file_info *fi) {
	(void) fi;
	struct mnode *n = mfind(path);
	if (!n || n->isdir) return -ENOENT;
	char *d = realloc(n->data, size ? size : 1);
	if (size && !d) return -ENOMEM;
	if ((size_t) size > n->size && d) memset(d + n->size, 0, size - n->size);
	n->data = d; n->size = size;
	return 0;
}
static const struct fuse_operations MM_OPS = {
	.getattr = mm_getattr, .readdir = mm_readdir, .open = mm_open, .read = mm_read,
	.write = mm_write, .create = mm_create, .mkdir = mm_mkdir, .unlink = mm_unlink,
	.rmdir = mm_rmdir, .truncate = mm_truncate,
};

/* custom_io: plain read/writev on the socketpair fd */
static ssize_t io_writev(int fd, struct iovec *iov, int count, void *u) { (void) u; return writev(fd, iov, count); }
static ssize_t io_read(int fd, void *buf, size_t len, void *u) { (void) u; return read(fd, buf, len); }

static int run_daemon(int fd) {
	mnew("/", 1, 0755);
	struct mnode *h = mnew("/hello", 0, 0644);
	const char *msg = "Hello World!\n";
	h->size = strlen(msg); h->data = malloc(h->size); memcpy(h->data, msg, h->size);
	mnew("/sub", 1, 0755);

	struct fuse_args args = FUSE_ARGS_INIT(0, NULL);
	fuse_opt_add_arg(&args, "fused-memfs");
	struct fuse *fu = fuse_new(&args, &MM_OPS, sizeof MM_OPS, NULL);
	if (!fu) { fprintf(stderr, "daemon: fuse_new failed\n"); return 1; }
	struct fuse_session *se = fuse_get_session(fu);
	struct fuse_custom_io io = { .writev = io_writev, .read = io_read, .splice_receive = NULL, .splice_send = NULL };
	if (fuse_session_custom_io(se, &io, fd) != 0) { fprintf(stderr, "daemon: custom_io failed\n"); return 1; }
	int rc = fuse_session_loop(se);
	fuse_destroy(fu);
	return rc;
}

/* ───────────────────────────── parent: drive fused ────────────────────────── */

static int FAILED = 0;
#define CHECK(cond, msg) do { if (cond) { printf("  ok   %s\n", msg); } else { printf("  FAIL %s\n", msg); FAILED = 1; } } while (0)

struct diracc { int hello, sub, n; };
static void on_ent(void *ctx, const char *name, unsigned type) {
	(void) type;
	struct diracc *d = ctx;
	d->n++;
	if (strcmp(name, "hello") == 0) d->hello = 1;
	if (strcmp(name, "sub") == 0)   d->sub = 1;
}

int main(void) {
	setvbuf(stdout, NULL, _IONBF, 0);
	int sv[2];
	if (socketpair(AF_UNIX, SOCK_SEQPACKET, 0, sv) != 0) { perror("socketpair"); return 2; }

	pid_t pid = fork();
	if (pid < 0) { perror("fork"); return 2; }
	if (pid == 0) { close(sv[0]); _exit(run_daemon(sv[1])); }
	close(sv[1]);

	fused_t *f = fused_new(sv[0], 0, 0);
	CHECK(f != NULL, "fused_new");
	if (!f) return 2;

	int rc = fused_init(f);
	CHECK(rc == 0, "fused_init (FUSE_INIT handshake with libfuse)");

	struct stat st;
	rc = fused_getattr(f, "/", &st);
	CHECK(rc == 0 && S_ISDIR(st.st_mode), "getattr / is a directory");

	rc = fused_getattr(f, "/hello", &st);
	CHECK(rc == 0 && S_ISREG(st.st_mode) && st.st_size == 13, "getattr /hello (regular, 13 bytes)");

	uint64_t fh = 0;
	rc = fused_open(f, "/hello", O_RDONLY, &fh);
	CHECK(rc == 0, "open /hello");
	char buf[64]; memset(buf, 0, sizeof buf);
	rc = fused_read(f, "/hello", fh, buf, sizeof buf, 0);
	CHECK(rc == 13 && memcmp(buf, "Hello World!\n", 13) == 0, "read /hello == \"Hello World!\\n\"");
	fused_release(f, "/hello", fh);

	struct diracc d = {0,0,0};
	rc = fused_readdir(f, "/", on_ent, &d);
	CHECK(rc == 0 && d.hello && d.sub, "readdir / lists hello + sub");

	/* write path */
	fh = 0;
	rc = fused_create(f, "/new.txt", O_RDWR, 0644, &fh);
	CHECK(rc == 0, "create /new.txt");
	rc = fused_write(f, "/new.txt", fh, "abcde", 5, 0);
	CHECK(rc == 5, "write 5 bytes to /new.txt");
	fused_release(f, "/new.txt", fh);
	memset(buf, 0, sizeof buf);
	rc = fused_open(f, "/new.txt", O_RDONLY, &fh);
	rc = fused_read(f, "/new.txt", fh, buf, sizeof buf, 0);
	CHECK(rc == 5 && memcmp(buf, "abcde", 5) == 0, "read-back /new.txt == \"abcde\"");
	fused_release(f, "/new.txt", fh);

	rc = fused_mkdir(f, "/d2", 0755);
	CHECK(rc == 0, "mkdir /d2");
	rc = fused_getattr(f, "/d2", &st);
	CHECK(rc == 0 && S_ISDIR(st.st_mode), "getattr /d2 is a directory");

	rc = fused_truncate(f, "/new.txt", 2);
	CHECK(rc == 0, "truncate /new.txt to 2");
	rc = fused_getattr(f, "/new.txt", &st);
	CHECK(rc == 0 && st.st_size == 2, "getattr /new.txt size == 2 after truncate");

	rc = fused_unlink(f, "/new.txt");
	CHECK(rc == 0, "unlink /new.txt");
	rc = fused_getattr(f, "/new.txt", &st);
	CHECK(rc == -ENOENT, "getattr /new.txt -> ENOENT after unlink");

	fused_destroy(f);          /* sends FUSE_DESTROY -> daemon loop exits */
	close(sv[0]);

	int status = 0;
	waitpid(pid, &status, 0);

	printf(FAILED ? "fused: FAIL\n" : "fused: all checks passed\n");
	return FAILED ? 1 : 0;
}

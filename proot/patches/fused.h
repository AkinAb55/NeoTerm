/*
 * fused.h — the FUSE "kernel" engine, app/proot side.
 *
 * A FUSE filesystem is a userspace daemon (sshfs, rclone, gocryptfs, ntfs-3g,
 * the mount inside an AppImage, …) plus the kernel FUSE driver. Under proot
 * there is no kernel FUSE the guest can use (open("/dev/fuse")+mount(2) need
 * real root), so `fused` PLAYS THE KERNEL: it speaks the FUSE wire protocol
 * (fuse_kernel.h) to the daemon over the channel fd (our /dev/fuse socketpair
 * end) and exposes a small path-based VFS API the proot redirect calls to serve
 * the guest's syscalls under the mountpoint — exactly the role ukfsd plays for
 * block filesystems, but the backend is the FUSE daemon instead of a FS driver.
 *
 * One fused instance per FUSE mount (single channel). All calls are synchronous:
 * build request -> write(channel) -> read(channel) reply -> translate. The
 * channel is a SEQPACKET socket so each datagram is one FUSE message; the
 * daemon's own read()/write() on it are serviced by the kernel, not proot.
 *
 * Paths passed in are relative to the mount root and absolute-style, e.g. "/",
 * "/dir", "/dir/file". Every op returns 0 on success or a negative errno.
 */
#ifndef FUSED_H
#define FUSED_H

#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>

typedef struct fused fused_t;

/* Create an engine over an already-connected channel fd (our end of the
 * daemon's /dev/fuse). uid/gid are stamped into every request header (the
 * daemon may use them; pass the guest's effective ids). Returns NULL on OOM. */
fused_t *fused_new(int chan_fd, uint32_t uid, uint32_t gid);

/* Install a "wait until the channel fd is readable" callback. Under proot the
 * FUSE daemon is a ptraced tracee whose channel read()/write() trap at sysexit,
 * so the single-threaded tracer must NOT block in read(channel) — it has to pump
 * the event loop (servicing the daemon's stops) until a reply lands. This hook
 * lets the redirect supply that pump; with no hook (e.g. the host test, where the
 * daemon is an ordinary process) fused just blocks in read(). */
void fused_set_wait(fused_t *f, void (*wait_readable)(int fd, void *ctx), void *ctx);

/* FUSE_INIT handshake: negotiate version/flags with the daemon. Must be called
 * once before any op. Returns 0 on success, negative errno on failure. */
int fused_init(fused_t *f);

/* Tear down: sends FUSE_DESTROY (best effort), frees the node cache. Does NOT
 * close the channel fd (the caller owns it). */
void fused_destroy(fused_t *f);

/* ---- read-side VFS ops ---- */
int fused_getattr(fused_t *f, const char *path, struct stat *st);
int fused_readlink(fused_t *f, const char *path, char *buf, size_t cap); /* NUL-terminates */
int fused_access (fused_t *f, const char *path, int mask);
int fused_statfs (fused_t *f, const char *path, struct statvfs *out);

/* Open/close a regular file. *fh receives the daemon's file handle. */
int fused_open   (fused_t *f, const char *path, int flags, uint64_t *fh);
int fused_read   (fused_t *f, const char *path, uint64_t fh, void *buf, size_t size, off_t off);
int fused_write  (fused_t *f, const char *path, uint64_t fh, const void *buf, size_t size, off_t off);
int fused_flush  (fused_t *f, const char *path, uint64_t fh);
int fused_fsync  (fused_t *f, const char *path, uint64_t fh, int datasync);
int fused_release(fused_t *f, const char *path, uint64_t fh);

/* Stateless pread/pwrite: open the path, do the I/O at `off`, release. Convenient
 * for a path-based redirect that doesn't keep the guest's fh around. Return the
 * byte count (>=0) or a negative errno. */
int fused_pread (fused_t *f, const char *path, void *buf, size_t size, off_t off);
int fused_pwrite(fused_t *f, const char *path, const void *buf, size_t size, off_t off);

/* Directory listing. `emit` is called once per entry with (ctx, name, type),
 * where type is a DT_* value (DT_REG/DT_DIR/...) or 0 if unknown. "." and ".."
 * are passed through if the daemon returns them. */
int fused_readdir(fused_t *f, const char *path,
                  void (*emit)(void *ctx, const char *name, unsigned type), void *ctx);

/* ---- write-side VFS ops ---- */
int fused_create (fused_t *f, const char *path, int flags, mode_t mode, uint64_t *fh);
int fused_mkdir  (fused_t *f, const char *path, mode_t mode);
int fused_unlink (fused_t *f, const char *path);
int fused_rmdir  (fused_t *f, const char *path);
int fused_rename (fused_t *f, const char *from, const char *to);
int fused_symlink(fused_t *f, const char *target, const char *linkpath);
int fused_truncate(fused_t *f, const char *path, off_t size);
int fused_chmod  (fused_t *f, const char *path, mode_t mode);
int fused_chown  (fused_t *f, const char *path, uid_t uid, gid_t gid);
int fused_utimens(fused_t *f, const char *path, const struct timespec tv[2]);

#endif /* FUSED_H */

/* uKernel — block-over-socket backend client (io.neoterm.block).
 *
 * Kept in its own translation unit, deliberately free of the fake kernel
 * headers (<linux/...>), because the real libc <sys/socket.h> it needs pulls
 * <asm-generic/posix_types.h>, whose __kernel_fsid_t collides with the fake
 * <linux/fs.h> definition on glibc. vfs.c calls these via plain extern decls.
 *
 * Protocol (matches BlockBridge / the proot block proxy), newline-framed:
 *   SIZE\n               -> OK <bytes> <sector>\n
 *   READ <off> <len>\n   -> OK <n>\n + n bytes
 *   WRITE <off> <len>\n + <len> bytes -> OK\n
 *   FLUSH\n              -> OK\n
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

static int g_bsock = -1;

static int bsk_wn(const void *b, size_t n)
{ size_t o = 0; while (o < n) { ssize_t r = write(g_bsock, (const char *)b + o, n - o); if (r <= 0) return -1; o += (size_t)r; } return 0; }
static int bsk_rn(void *b, size_t n)
{ size_t o = 0; while (o < n) { ssize_t r = read(g_bsock, (char *)b + o, n - o); if (r <= 0) return -1; o += (size_t)r; } return 0; }
static int bsk_rl(char *b, size_t bs)
{ size_t o = 0; while (o + 1 < bs) { char c; if (bsk_rn(&c, 1) < 0) return -1; if (c == '\n') { b[o] = 0; return (int)o; } b[o++] = c; } b[o] = 0; return (int)o; }

/* Connect to the abstract unix socket <name>. 0 on success. */
int bsock_open(const char *name)
{
	int s = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
	if (s < 0) return -1;
	struct sockaddr_un a; memset(&a, 0, sizeof a); a.sun_family = AF_UNIX;
	a.sun_path[0] = '\0'; strncpy(a.sun_path + 1, name, sizeof(a.sun_path) - 2);
	socklen_t len = (socklen_t)(sizeof(a.sun_family) + 1 + strlen(name));
	if (connect(s, (struct sockaddr *) &a, len) < 0) { close(s); return -1; }
	g_bsock = s;
	return 0;
}
void bsock_close(void) { if (g_bsock >= 0) { close(g_bsock); g_bsock = -1; } }

ssize_t bsock_pread(void *buf, size_t len, off_t off)
{
	if (g_bsock < 0) return -1;
	char req[64]; int rl = snprintf(req, sizeof req, "READ %lld %zu\n", (long long) off, len);
	if (bsk_wn(req, (size_t) rl) < 0) return -1;
	char line[64]; if (bsk_rl(line, sizeof line) < 0) return -1;
	size_t n; if (sscanf(line, "OK %zu", &n) != 1) return -1;
	if (n > len) n = len;
	if (n && bsk_rn(buf, n) < 0) return -1;
	return (ssize_t) n;
}
ssize_t bsock_pwrite(const void *buf, size_t len, off_t off)
{
	if (g_bsock < 0) return -1;
	char req[64]; int rl = snprintf(req, sizeof req, "WRITE %lld %zu\n", (long long) off, len);
	if (bsk_wn(req, (size_t) rl) < 0 || bsk_wn(buf, len) < 0) return -1;
	char line[16]; if (bsk_rl(line, sizeof line) < 0) return -1;
	return (line[0] == 'O' && line[1] == 'K') ? (ssize_t) len : -1;
}
long long bsock_size(void)
{
	if (g_bsock < 0) return -1;
	if (bsk_wn("SIZE\n", 5) < 0) return -1;
	char line[64]; if (bsk_rl(line, sizeof line) < 0) return -1;
	long long sz; int sec;
	if (sscanf(line, "OK %lld %d", &sz, &sec) < 1) return -1;
	return sz;
}
void bsock_flush(void)
{
	if (g_bsock < 0) return;
	if (bsk_wn("FLUSH\n", 6) == 0) { char l[16]; (void) bsk_rl(l, sizeof l); }
}

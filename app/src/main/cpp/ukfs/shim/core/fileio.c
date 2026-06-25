/* uKernel — fájl-I/O, random, init_net és egyéb maradék shim-implementációk. */
#include <linux/fs.h>
#include <linux/random.h>
#include <linux/netdevice.h>
#include <linux/kthread.h>
#include <net/ieee80211_radiotap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <linux/errno.h>

/* ===== init_net ===== */
struct net init_net;

/* ===== random ===== */
static unsigned long rnd_state = 0x12345678;
unsigned int get_random_u32(void)
{ rnd_state = rnd_state * 6364136223846793005UL + 1442695040888963407UL; return (unsigned int)(rnd_state >> 32); }
void get_random_bytes(void *buf, int n) { unsigned char *p = buf; for (int i = 0; i < n; i++) p[i] = (unsigned char)get_random_u32(); }

/* ===== fájl-I/O (firmware/efuse betöltés) — valós POSIX fájlokra ===== */
struct file *filp_open(const char *name, int flags, int mode)
{
	int fd = open(name, flags, mode);
	if (fd < 0) return (struct file *)ERR_PTR(-2 /*ENOENT*/);
	struct file *f = calloc(1, sizeof(*f));
	f->private_data = (void *)(long)fd;
	f->f_flags = flags;
	return f;
}
int filp_close(struct file *f, void *id)
{ (void)id; if (!f) return 0; close((int)(long)f->private_data); free(f); return 0; }
long kernel_read(struct file *f, void *buf, unsigned long count, loff_t *pos)
{
	int fd = (int)(long)f->private_data;
	if (pos) lseek(fd, *pos, SEEK_SET);
	ssize_t r = read(fd, buf, count);
	if (r > 0 && pos) *pos += r;
	return r;
}
long kernel_write(struct file *f, const void *buf, unsigned long count, loff_t *pos)
{
	int fd = (int)(long)f->private_data;
	if (pos) lseek(fd, *pos, SEEK_SET);
	ssize_t r = write(fd, buf, count);
	if (r > 0 && pos) *pos += r;
	return r;
}
long vfs_read(struct file *f, char __user *buf, unsigned long count, loff_t *pos)  { return kernel_read(f, buf, count, pos); }
long vfs_write(struct file *f, const char __user *buf, unsigned long count, loff_t *pos) { return kernel_write(f, buf, count, pos); }

/* ===== netdev név-allokáció ===== */
int dev_alloc_name(struct net_device *dev, const char *name)
{
	static int idx;
	if (strchr(name, '%')) snprintf(dev->name, IFNAMSIZ, name, idx++);
	else snprintf(dev->name, IFNAMSIZ, "%s", name);
	return 0;
}

/* ===== kthread ===== */
void kthread_complete_and_exit(struct completion *c, long code) { (void)c; (void)code; pthread_exit(NULL); }
void complete_and_exit(struct completion *c, long code) { kthread_complete_and_exit(c, code); }

/* ===== radiotap iterator (egyszerűsített) ===== */
int ieee80211_radiotap_iterator_init(struct ieee80211_radiotap_iterator *it,
		struct ieee80211_radiotap_header *h, int len, const void *ns)
{ (void)ns; memset(it, 0, sizeof(*it)); it->_rtheader = h; it->_max_length = len; return 0; }
int ieee80211_radiotap_iterator_next(struct ieee80211_radiotap_iterator *it) { (void)it; return -1 /*nincs több mező*/; }

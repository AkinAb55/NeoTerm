/* uKernel — char-device registry és a proxy file-API.
 * A driver register_chrdev/ukernel_register_cdev-del jegyzi a file_operations-t;
 * a uServer proxy a ukernel_file_* függvényekkel hívja a kliens nevében. */
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include "ukernel/runtime.h"
#include <stdlib.h>
#include <string.h>

#define MAX_CDEV 32
struct cdev_slot {
	char name[48];
	const struct file_operations *fops;
	void *drvdata;
	unsigned major;
	int used;
};
static struct cdev_slot g_cdev[MAX_CDEV];
static unsigned g_next_major = 240;  /* dinamikus tartomány */

int ukernel_register_cdev(const char *name, const struct file_operations *fops, void *drvdata)
{
	for (int i = 0; i < MAX_CDEV; i++) {
		if (!g_cdev[i].used) {
			g_cdev[i].used = 1;
			snprintf(g_cdev[i].name, sizeof(g_cdev[i].name), "%s", name);
			g_cdev[i].fops = fops; g_cdev[i].drvdata = drvdata;
			g_cdev[i].major = g_next_major++;
			printk(KERN_INFO "uKernel/fs: cdev regisztrálva: '%s' (major %u)\n", name, g_cdev[i].major);
			return 0;
		}
	}
	return -1;
}
void ukernel_unregister_cdev(const char *name)
{
	for (int i = 0; i < MAX_CDEV; i++)
		if (g_cdev[i].used && strcmp(g_cdev[i].name, name) == 0) g_cdev[i].used = 0;
}

int register_chrdev(unsigned int major, const char *name, const struct file_operations *fops)
{
	int r = ukernel_register_cdev(name, fops, NULL);
	return r ? r : (major ? (int)major : (int)g_cdev[0].major);
}
void unregister_chrdev(unsigned int major, const char *name) { (void)major; ukernel_unregister_cdev(name); }

static struct cdev_slot *find_cdev(const char *name)
{
	for (int i = 0; i < MAX_CDEV; i++)
		if (g_cdev[i].used && strcmp(g_cdev[i].name, name) == 0) return &g_cdev[i];
	return NULL;
}

/* ===== proxy file-API ===== */
struct uk_open_file { struct file f; struct inode ino; const struct file_operations *fops; };

size_t ukernel_cdev_count(void)
{ size_t n = 0; for (int i = 0; i < MAX_CDEV; i++) if (g_cdev[i].used) n++; return n; }

const char *ukernel_cdev_name(size_t idx)
{
	size_t n = 0;
	for (int i = 0; i < MAX_CDEV; i++)
		if (g_cdev[i].used) { if (n == idx) return g_cdev[i].name; n++; }
	return NULL;
}

void *ukernel_file_open(const char *name, unsigned int flags)
{
	struct cdev_slot *s = find_cdev(name);
	if (!s) { printk(KERN_ERR "uKernel/fs: nincs cdev: '%s'\n", name); return NULL; }
	struct uk_open_file *of = calloc(1, sizeof(*of));
	of->fops = s->fops;
	of->f.f_op = s->fops;
	of->f.f_flags = flags;
	of->f.f_inode = &of->ino;
	of->ino.i_private = s->drvdata;
	of->f.private_data = NULL;
	if (s->fops && s->fops->open) {
		int r = s->fops->open(&of->ino, &of->f);
		if (r) { printk(KERN_ERR "uKernel/fs: open('%s') -> %d\n", name, r); free(of); return NULL; }
	}
	return of;
}

long ukernel_file_read(void *fh, void *buf, size_t n)
{
	struct uk_open_file *of = fh;
	if (!of->fops || !of->fops->read) return -ENOSYS;
	return of->fops->read(&of->f, buf, n, &of->f.f_pos);
}
long ukernel_file_write(void *fh, const void *buf, size_t n)
{
	struct uk_open_file *of = fh;
	if (!of->fops || !of->fops->write) return -ENOSYS;
	return of->fops->write(&of->f, buf, n, &of->f.f_pos);
}
long ukernel_file_ioctl(void *fh, unsigned int cmd, void *arg)
{
	struct uk_open_file *of = fh;
	if (!of->fops || !of->fops->unlocked_ioctl) return -ENOTTY;
	return of->fops->unlocked_ioctl(&of->f, cmd, (unsigned long)(uintptr_t)arg);
}
unsigned int ukernel_file_poll(void *fh)
{
	struct uk_open_file *of = fh;
	if (!of->fops || !of->fops->poll) return 0;
	return of->fops->poll(&of->f, NULL);
}
void ukernel_file_close(void *fh)
{
	struct uk_open_file *of = fh;
	if (of->fops && of->fops->release) of->fops->release(&of->ino, &of->f);
	free(of);
}

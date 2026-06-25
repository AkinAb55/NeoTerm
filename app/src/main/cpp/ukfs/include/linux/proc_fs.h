#ifndef _UK_LINUX_PROC_FS_H
#define _UK_LINUX_PROC_FS_H
#include <linux/types.h>
#include <linux/fs.h>
struct proc_dir_entry;
/* modern proc_ops (5.6+) */
struct proc_ops {
	unsigned int proc_flags;
	int     (*proc_open)(struct inode *, struct file *);
	ssize_t (*proc_read)(struct file *, char __user *, size_t, loff_t *);
	ssize_t (*proc_write)(struct file *, const char __user *, size_t, loff_t *);
	loff_t  (*proc_lseek)(struct file *, loff_t, int);
	int     (*proc_release)(struct inode *, struct file *);
};
struct proc_dir_entry *proc_mkdir(const char *name, struct proc_dir_entry *parent);
struct proc_dir_entry *proc_mkdir_data(const char *name, unsigned mode, struct proc_dir_entry *parent, void *data);
struct proc_dir_entry *proc_create_data(const char *name, unsigned mode, struct proc_dir_entry *parent,
		const struct proc_ops *ops, void *data);
void  remove_proc_entry(const char *name, struct proc_dir_entry *parent);
void *proc_get_parent_data(const struct inode *inode);
void *PDE_DATA(const struct inode *inode);
#define pde_data(inode) PDE_DATA(inode)
#endif

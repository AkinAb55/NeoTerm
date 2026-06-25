#ifndef _UK_LINUX_SEQ_FILE_H
#define _UK_LINUX_SEQ_FILE_H
#include <linux/types.h>
#include <linux/fs.h>
struct seq_file { struct file *file; void *private; size_t count; size_t size; char *buf; loff_t index; };
struct seq_operations {
	void *(*start)(struct seq_file *m, loff_t *pos);
	void  (*stop)(struct seq_file *m, void *v);
	void *(*next)(struct seq_file *m, void *v, loff_t *pos);
	int   (*show)(struct seq_file *m, void *v);
};
int  seq_printf(struct seq_file *m, const char *fmt, ...) __attribute__((format(printf,2,3)));
int  seq_puts(struct seq_file *m, const char *s);
int  seq_putc(struct seq_file *m, char c);
int  seq_write(struct seq_file *m, const void *data, size_t len);
int  seq_open(struct file *f, const struct seq_operations *op);
int  single_open(struct file *f, int (*show)(struct seq_file *, void *), void *data);
int  single_release(struct inode *i, struct file *f);
ssize_t seq_read(struct file *f, char __user *buf, size_t size, loff_t *ppos);
loff_t seq_lseek(struct file *f, loff_t off, int whence);
int  seq_release(struct inode *i, struct file *f);
#ifndef _UK_FILE_INODE
#define _UK_FILE_INODE
static inline struct inode *file_inode(const struct file *f) { return f->f_inode; }
#endif
#endif

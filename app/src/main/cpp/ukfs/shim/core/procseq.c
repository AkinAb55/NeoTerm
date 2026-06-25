/* uKernel — seq_file + proc_fs keretrendszer (a driver /proc debug-felülete).
 * Egyszerűsített: a seq_file egy növekvő pufferbe ír; a proc-entry-k egy
 * regiszterben élnek (a tényleges /proc megjelenítés nélkül). */
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* ===== seq_file ===== */
static void seq_ensure(struct seq_file *m, size_t add)
{
	if (m->count + add + 1 > m->size) {
		size_t ns = (m->size ? m->size : 4096);
		while (ns < m->count + add + 1) ns *= 2;
		m->buf = realloc(m->buf, ns); m->size = ns;
	}
}
int seq_printf(struct seq_file *m, const char *fmt, ...)
{
	va_list ap; va_start(ap, fmt);
	char tmp[1024]; int n = vsnprintf(tmp, sizeof(tmp), fmt, ap); va_end(ap);
	if (n < 0) return 0;
	seq_ensure(m, n); memcpy(m->buf + m->count, tmp, n); m->count += n; m->buf[m->count] = 0;
	return 0;
}
int seq_puts(struct seq_file *m, const char *s) { size_t n = strlen(s); seq_ensure(m, n); memcpy(m->buf + m->count, s, n); m->count += n; return 0; }
int seq_putc(struct seq_file *m, char c) { seq_ensure(m, 1); m->buf[m->count++] = c; return 0; }
int seq_write(struct seq_file *m, const void *data, size_t len) { seq_ensure(m, len); memcpy(m->buf + m->count, data, len); m->count += len; return 0; }

int single_open(struct file *f, int (*show)(struct seq_file *, void *), void *data)
{
	struct seq_file *m = calloc(1, sizeof(*m));
	m->private = data;
	f->private_data = m;
	if (show) show(m, NULL);
	return 0;
}
int seq_open(struct file *f, const struct seq_operations *op)
{
	struct seq_file *m = calloc(1, sizeof(*m));
	f->private_data = m;
	if (op && op->show) {
		loff_t pos = 0; void *v = op->start ? op->start(m, &pos) : NULL;
		while (v) { op->show(m, v); v = op->next ? op->next(m, v, &pos) : NULL; }
		if (op->stop) op->stop(m, NULL);
	}
	return 0;
}
ssize_t seq_read(struct file *f, char __user *buf, size_t size, loff_t *ppos)
{
	struct seq_file *m = f->private_data;
	if (!m || !m->buf) return 0;
	size_t off = ppos ? (size_t)*ppos : 0;
	if (off >= m->count) return 0;
	size_t n = m->count - off; if (n > size) n = size;
	memcpy((void *)buf, m->buf + off, n);
	if (ppos) *ppos += n;
	return n;
}
loff_t seq_lseek(struct file *f, loff_t off, int whence) { (void)f; (void)whence; return off; }
int seq_release(struct inode *i, struct file *f)
{ (void)i; struct seq_file *m = f->private_data; if (m) { free(m->buf); free(m); } return 0; }
int single_release(struct inode *i, struct file *f) { return seq_release(i, f); }

/* ===== proc_fs (regiszter, tényleges /proc nélkül) ===== */
struct proc_dir_entry { char name[64]; void *data; };

struct proc_dir_entry *proc_mkdir(const char *name, struct proc_dir_entry *parent)
{ (void)parent; struct proc_dir_entry *p = calloc(1, sizeof(*p)); snprintf(p->name, sizeof(p->name), "%s", name ? name : ""); return p; }
struct proc_dir_entry *proc_mkdir_data(const char *name, unsigned mode, struct proc_dir_entry *parent, void *data)
{ (void)mode; struct proc_dir_entry *p = proc_mkdir(name, parent); p->data = data; return p; }
struct proc_dir_entry *proc_create_data(const char *name, unsigned mode, struct proc_dir_entry *parent,
		const struct proc_ops *ops, void *data)
{ (void)mode; (void)parent; (void)ops; struct proc_dir_entry *p = calloc(1, sizeof(*p));
  snprintf(p->name, sizeof(p->name), "%s", name ? name : ""); p->data = data; return p; }
void remove_proc_entry(const char *name, struct proc_dir_entry *parent) { (void)name; (void)parent; }
void *proc_get_parent_data(const struct inode *inode) { (void)inode; return NULL; }
void *PDE_DATA(const struct inode *inode) { (void)inode; return NULL; }

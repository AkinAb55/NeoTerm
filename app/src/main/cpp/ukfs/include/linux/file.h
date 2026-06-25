#ifndef _UK_LINUX_FILE_H
#define _UK_LINUX_FILE_H
#include <linux/fs.h>
struct fd { struct file *file; unsigned int flags; };
struct file *fget(unsigned int fd);
void fput(struct file *file);
static inline struct file *fd_file(struct fd f) { return f.file; }
static inline bool fd_empty(struct fd f){ return !f.file; }
static inline struct fd uk_fd_get(int fd){ (void)fd; return (struct fd){0}; }
#endif

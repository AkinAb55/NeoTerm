/* uKernel hamis <linux/fsnotify.h> — fájlrendszer-értesítés no-op stubok. */
#ifndef _UK_LINUX_FSNOTIFY_H
#define _UK_LINUX_FSNOTIFY_H
struct inode;
struct dentry;
static inline void fsnotify_create(struct inode *dir, struct dentry *dentry) {}
static inline void fsnotify_mkdir(struct inode *dir, struct dentry *dentry) {}
static inline void fsnotify_unlink(struct inode *dir, struct dentry *dentry) {}
static inline void fsnotify_rmdir(struct inode *dir, struct dentry *dentry) {}
static inline void fsnotify_move(struct inode *o, struct inode *n, const void *q, int d, struct inode *t, struct dentry *m) {}
static inline void fsnotify_modify(void *file) {}
static inline void fsnotify_access(void *file) {}
#endif

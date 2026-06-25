/* uKernel hamis <linux/security.h> — LSM no-op stubok. */
#ifndef _UK_LINUX_SECURITY_H
#define _UK_LINUX_SECURITY_H
#include <linux/types.h>
struct inode;
struct dentry;
struct super_block;
struct fs_context;
struct qstr;

static inline int security_inode_init_security(struct inode *inode, struct inode *dir,
	const struct qstr *qstr, const void *initxattrs, void *fs_data) { return 0; }
static inline void security_inode_post_create(struct inode *dir, struct dentry *dentry, umode_t mode) {}
static inline int security_sb_mnt_opts_compat(struct super_block *sb, void *mnt_opts) { return 0; }
#endif

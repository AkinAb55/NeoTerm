/* uKernel hamis <linux/xattr.h>. */
#ifndef _UK_LINUX_XATTR_H
#define _UK_LINUX_XATTR_H
#include <linux/types.h>
struct inode; struct dentry; struct mnt_idmap; struct user_namespace;
struct xattr_handler {
	const char *name;
	const char *prefix;
	int flags;
	bool (*list)(struct dentry *dentry);
	int (*get)(const struct xattr_handler *, struct dentry *, struct inode *, const char *, void *, size_t);
	int (*set)(const struct xattr_handler *, struct mnt_idmap *, struct dentry *, struct inode *, const char *, const void *, size_t, int);
};
struct xattr { const char *name; void *value; size_t value_len; };
#define XATTR_CREATE  0x1
#define XATTR_REPLACE 0x2
#define XATTR_NAME_MAX 255
#define XATTR_SIZE_MAX 65536
#define XATTR_HURD_PREFIX "gnu."
#define XATTR_SYSTEM_PREFIX "system."
#define XATTR_USER_PREFIX "user."
#define XATTR_TRUSTED_PREFIX "trusted."
#define XATTR_SECURITY_PREFIX "security."
#define XATTR_NAME_POSIX_ACL_ACCESS  "system.posix_acl_access"
#define XATTR_NAME_POSIX_ACL_DEFAULT "system.posix_acl_default"
ssize_t generic_listxattr(struct dentry *dentry, char *buffer, size_t size);
int xattr_full_name(const struct xattr_handler *, const char *);
static inline const char *xattr_prefix(const struct xattr_handler *handler) { return handler->prefix ?: handler->name; }
static inline bool xattr_handler_can_list(const struct xattr_handler *handler, struct dentry *dentry) { return handler && (!handler->list || handler->list(dentry)); }
#endif

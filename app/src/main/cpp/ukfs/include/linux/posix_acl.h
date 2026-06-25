#ifndef _UK_LINUX_POSIX_ACL_H
#define _UK_LINUX_POSIX_ACL_H
#include <linux/types.h>
struct posix_acl_entry { short e_tag; unsigned short e_perm; union { kuid_t e_uid; kgid_t e_gid; }; };
struct posix_acl { atomic_t a_refcount; unsigned int a_count; struct posix_acl_entry a_entries[]; };
#define ACL_USER_OBJ 0x01
#define ACL_USER 0x02
#define ACL_GROUP_OBJ 0x04
#define ACL_GROUP 0x08
#define ACL_MASK 0x10
#define ACL_OTHER 0x20
struct posix_acl *posix_acl_alloc(unsigned int count, gfp_t flags);
int posix_acl_create(struct inode *inode, umode_t *mode, struct posix_acl **default_acl, struct posix_acl **acl);
void posix_acl_release(struct posix_acl *acl);
int posix_acl_equiv_mode(const struct posix_acl *acl, umode_t *mode_p);
#define ACL_NOT_CACHED ((void *)(-1))
#define ACL_DONT_CACHE ((void *)(-3))
struct inode; struct mnt_idmap; struct dentry;
static inline struct posix_acl *get_inode_acl(struct inode *i, int type) { (void)i;(void)type; return NULL; }
static inline void forget_cached_acl(struct inode *inode, int type) { (void)inode;(void)type; }
static inline void forget_all_cached_acls(struct inode *inode) { (void)inode; }
int posix_acl_chmod(struct mnt_idmap *idmap, struct dentry *dentry, umode_t mode);
struct posix_acl *posix_acl_clone(const struct posix_acl *acl, gfp_t flags);
#define ACL_TYPE_ACCESS 0x8000
#define ACL_TYPE_DEFAULT 0x4000
void set_cached_acl(struct inode *inode, int type, struct posix_acl *acl);
struct posix_acl *posix_acl_from_xattr(struct user_namespace *user_ns, const void *value, size_t size);
int posix_acl_to_xattr(struct user_namespace *user_ns, const struct posix_acl *acl, void *buffer, size_t size);
#endif

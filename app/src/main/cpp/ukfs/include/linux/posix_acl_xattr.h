#ifndef _UK_LINUX_POSIX_ACL_XATTR_H
#define _UK_LINUX_POSIX_ACL_XATTR_H
#include <linux/xattr.h>
#include <linux/posix_acl.h>
#define XATTR_NAME_POSIX_ACL_ACCESS_UK "system.posix_acl_access"
extern const struct xattr_handler nop_posix_acl_access;
extern const struct xattr_handler nop_posix_acl_default;

/* a POSIX ACL xattr-bináris formátuma (uapi) — a posix_acl <-> xattr konverzióhoz */
#define POSIX_ACL_XATTR_VERSION 0x0002
#define ACL_UNDEFINED_ID (-1)
struct posix_acl_xattr_entry  { __le16 e_tag; __le16 e_perm; __le32 e_id; };
struct posix_acl_xattr_header { __le32 a_version; };
static inline size_t posix_acl_xattr_size(int count)
{ return sizeof(struct posix_acl_xattr_header) + count * sizeof(struct posix_acl_xattr_entry); }
static inline int posix_acl_xattr_count(size_t size)
{
	if (size < sizeof(struct posix_acl_xattr_header)) return -1;
	size -= sizeof(struct posix_acl_xattr_header);
	if (size % sizeof(struct posix_acl_xattr_entry)) return -1;
	return (int)(size / sizeof(struct posix_acl_xattr_entry));
}
#endif

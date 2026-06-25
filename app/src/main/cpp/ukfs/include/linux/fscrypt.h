/* uKernel hamis <linux/fscrypt.h> — titkosítás kikapcsolva (stub). */
#ifndef _UK_LINUX_FSCRYPT_H
#define _UK_LINUX_FSCRYPT_H
#include <linux/types.h>
struct fscrypt_dummy_policy { void *policy; };
struct fscrypt_str { unsigned char *name; u32 len; };
struct fscrypt_name { const struct qstr *usr_fname; struct fscrypt_str disk_name; u32 hash; u32 minor_hash; struct fscrypt_str crypto_buf; bool is_nokey_name; };
#define FSTR_INIT(n, l) { .name = n, .len = l }
#define fname_name(p) ((p)->disk_name.name)
#define fname_len(p)  ((p)->disk_name.len)
static inline bool fscrypt_has_encryption_key(const struct inode *inode) { (void)inode; return false; }
static inline bool IS_ENCRYPTED(const struct inode *inode) { (void)inode; return false; }
static inline int fscrypt_prepare_lookup(struct inode *dir, struct dentry *dentry, struct fscrypt_name *fname) { (void)dir;(void)dentry; memset(fname,0,sizeof(*fname)); return 0; }
static inline void fscrypt_free_filename(struct fscrypt_name *fname) { (void)fname; }
/* fscrypt_match_name (titkosítás nélkül): sima név-összehasonlítás. KELL — különben az
 * ext4 a no-op stubra oldódik (mindig false) és a könyvtár-keresés sosem talál! */
static inline bool fscrypt_match_name(const struct fscrypt_name *fname, const u8 *de_name, u32 de_name_len)
{
	if (de_name_len != fname->disk_name.len) return false;
	return !memcmp(de_name, fname->disk_name.name, fname->disk_name.len);
}
/* fscrypt_prepare_symlink (titkosítás nélkül): a disk_link a NYERS célra mutat (+1 a NUL).
 * KELL — különben az ext4_symlink a no-op stubra oldódik → a disk_link kitöltetlen (garbage
 * name/len) → a fast-symlink memcpy/branch SEGFAULT-ol. */
static inline int fscrypt_prepare_symlink(struct inode *dir, const char *target, unsigned int len, unsigned int max_len, struct fscrypt_str *disk_link)
{
	(void)dir;
	disk_link->name = (unsigned char *)target;
	disk_link->len = len + 1;
	if (disk_link->len > max_len) return -36;   /* -ENAMETOOLONG */
	return 0;
}
static inline int __fscrypt_encrypt_symlink(struct inode *inode, const char *target, unsigned int len, struct fscrypt_str *disk_link) { (void)inode;(void)target;(void)len;(void)disk_link; return 0; }
static inline int fscrypt_get_encryption_info(struct inode *inode) { (void)inode; return 0; }
static inline bool fscrypt_needs_contents_encryption(const struct inode *inode) { (void)inode; return false; }
#include <linux/errno.h>
static inline const char *fscrypt_get_symlink(struct inode *inode, const void *caddr, unsigned int max_size, struct delayed_call *done) { (void)inode;(void)caddr;(void)max_size;(void)done; return ERR_PTR(-95); }
static inline int fscrypt_symlink_getattr(const struct path *path, struct kstat *stat) { (void)path;(void)stat; return -95; }
static inline struct folio *fscrypt_pagecache_folio(const struct folio *f) { (void)f; return 0; }
static inline bool fscrypt_is_bounce_folio(const struct folio *folio) { (void)folio; return false; }
static inline struct page *fscrypt_encrypt_pagecache_blocks(struct folio *folio, size_t len, size_t offs, gfp_t gfp) { (void)folio;(void)len;(void)offs;(void)gfp; return ERR_PTR(-95); }
static inline void fscrypt_free_bounce_page(struct page *bounce_page) { (void)bounce_page; }
static inline bool fscrypt_inode_uses_fs_layer_crypto(const struct inode *inode) { (void)inode; return false; }
static inline void fscrypt_set_bio_crypt_ctx(struct bio *bio, const struct inode *inode, u64 first_lblk, gfp_t gfp_mask) {}
static inline bool fscrypt_mergeable_bio(struct bio *bio, const struct inode *inode, u64 next_lblk) { return true; }
static inline int fscrypt_setup_filename(struct inode *dir, const struct qstr *iname, int lookup, struct fscrypt_name *fname) { (void)dir; memset(fname,0,sizeof(*fname)); fname->disk_name.name=(unsigned char*)iname->name; fname->disk_name.len=iname->len; return 0; }
struct fscrypt_policy_v1 { __u8 version; __u8 contents_encryption_mode; __u8 filenames_encryption_mode; __u8 flags; __u8 master_key_descriptor[8]; };
struct fscrypt_key_specifier { __u32 type; __u32 __reserved; union { __u8 __reserved2[32]; __u8 descriptor[8]; __u8 identifier[16]; } u; };
struct fscrypt_add_key_arg { struct fscrypt_key_specifier key_spec; __u32 raw_size; __u32 key_id; __u32 __reserved[8]; __u8 raw[]; };
struct fscrypt_remove_key_arg { struct fscrypt_key_specifier key_spec; __u32 removal_status_flags; __u32 __reserved[5]; };
struct fscrypt_get_key_status_arg { struct fscrypt_key_specifier key_spec; __u32 __reserved[6]; __u32 status; __u32 status_flags; __u32 user_count; __u32 __out_reserved[13]; };
#define FS_IOC_SET_ENCRYPTION_POLICY _IOR('f', 19, struct fscrypt_policy_v1)
#define FS_IOC_GET_ENCRYPTION_PWSALT _IOW('f', 20, char[16])
#define FS_IOC_GET_ENCRYPTION_POLICY _IOW('f', 21, struct fscrypt_policy_v1)
#define FS_IOC_GET_ENCRYPTION_POLICY_EX _IOWR('f', 22, char[9])
#define FS_IOC_ADD_ENCRYPTION_KEY _IOWR('f', 23, struct fscrypt_add_key_arg)
#define FS_IOC_REMOVE_ENCRYPTION_KEY _IOWR('f', 24, struct fscrypt_remove_key_arg)
#define FS_IOC_REMOVE_ENCRYPTION_KEY_ALL_USERS _IOWR('f', 25, struct fscrypt_remove_key_arg)
#define FS_IOC_GET_ENCRYPTION_KEY_STATUS _IOWR('f', 26, struct fscrypt_get_key_status_arg)
#define FS_IOC_GET_ENCRYPTION_NONCE _IOR('f', 27, char[16])
#endif

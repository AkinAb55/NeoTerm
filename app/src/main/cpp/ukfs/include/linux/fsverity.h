#ifndef _UK_LINUX_FSVERITY_H
#define _UK_LINUX_FSVERITY_H
#include <linux/types.h>
struct inode;
static inline bool fsverity_active(const struct inode *inode) { (void)inode; return false; }
static inline bool IS_VERITY(const struct inode *inode) { (void)inode; return false; }
struct fsverity_info;
static inline struct fsverity_info *fsverity_get_info(const struct inode *inode) { (void)inode; return 0; }
static inline void fsverity_readahead(struct fsverity_info *vi, pgoff_t index, unsigned long nr_pages) {}
static inline int fsverity_file_open(struct inode *inode, struct file *filp) { return 0; }
static inline int fsverity_prepare_setattr(struct dentry *dentry, struct iattr *attr) { return 0; }
struct fsverity_enable_arg { __u32 version; __u32 hash_algorithm; __u32 block_size; __u32 salt_size; __u64 salt_ptr; __u32 sig_size; __u32 __reserved1; __u64 sig_ptr; __u64 __reserved2[11]; };
struct fsverity_digest { __u16 digest_algorithm; __u16 digest_size; __u8 digest[]; };
struct fsverity_read_metadata_arg { __u64 metadata_type; __u64 offset; __u64 length; __u64 buf_ptr; __u64 __reserved; };
#define FS_IOC_ENABLE_VERITY _IOW('f', 133, struct fsverity_enable_arg)
#define FS_IOC_MEASURE_VERITY _IOWR('f', 134, struct fsverity_digest)
#define FS_IOC_READ_VERITY_METADATA _IOWR('f', 135, struct fsverity_read_metadata_arg)
#endif

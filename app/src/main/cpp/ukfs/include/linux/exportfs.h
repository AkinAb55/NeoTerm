/* uKernel hamis <linux/exportfs.h> — NFS-export felület stub. */
#ifndef _UK_LINUX_EXPORTFS_H
#define _UK_LINUX_EXPORTFS_H
#include <linux/types.h>
struct inode;
struct dentry;
struct super_block;
struct fid;

enum fid_type {
	FILEID_INVALID = 0xff, FILEID_INO32_GEN = 1, FILEID_INO32_GEN_PARENT = 2,
	FILEID_FAT_WITHOUT_PARENT = 0x71, FILEID_FAT_WITH_PARENT = 0x72,
};
#define FAT_FID_SIZE_WITHOUT_PARENT 2
#define FAT_FID_SIZE_WITH_PARENT 4

struct export_operations {
	int (*encode_fh)(struct inode *inode, __u32 *fh, int *max_len, struct inode *parent);
	struct dentry *(*fh_to_dentry)(struct super_block *sb, struct fid *fid, int fh_len, int fh_type);
	struct dentry *(*fh_to_parent)(struct super_block *sb, struct fid *fid, int fh_len, int fh_type);
	int (*get_name)(struct dentry *parent, char *name, struct dentry *child);
	struct dentry *(*get_parent)(struct dentry *child);
	int (*commit_metadata)(struct inode *inode);
	int (*nr_addr_blocks)(void);
	long flags;
};
#define EXPORT_FH_FID 0x2

struct fid {
	union {
		struct { u32 ino; u32 gen; u32 parent_ino; u32 parent_gen; } i32;
		__u32 raw[6];
	};
};

int generic_encode_ino32_fh(struct inode *inode, __u32 *fh, int *max_len, struct inode *parent);
struct dentry *generic_fh_to_dentry(struct super_block *sb, struct fid *fid, int fh_len, int fh_type,
	struct inode *(*get_inode)(struct super_block *sb, u64 ino, u32 gen));
struct dentry *generic_fh_to_parent(struct super_block *sb, struct fid *fid, int fh_len, int fh_type,
	struct inode *(*get_inode)(struct super_block *sb, u64 ino, u32 gen));
#endif

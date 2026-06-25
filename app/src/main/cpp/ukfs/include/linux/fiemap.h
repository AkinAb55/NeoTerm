/* uKernel hamis <linux/fiemap.h>. */
#ifndef _UK_LINUX_FIEMAP_H
#define _UK_LINUX_FIEMAP_H
#include <linux/types.h>
#include <linux/compiler.h>
struct fiemap_extent { u64 fe_logical; u64 fe_physical; u64 fe_length; u64 fe_reserved64[2]; u32 fe_flags; u32 fe_reserved[3]; };
struct fiemap { __u64 fm_start; __u64 fm_length; __u32 fm_flags; __u32 fm_mapped_extents; __u32 fm_extent_count; __u32 fm_reserved; struct fiemap_extent fm_extents[]; };
struct fiemap_extent_info {
	unsigned int fi_flags;
	unsigned int fi_extents_mapped;
	unsigned int fi_extents_max;
	struct fiemap_extent __user *fi_extents_start;
};
#define FIEMAP_FLAG_SYNC 0x00000001
#define FIEMAP_FLAG_XATTR 0x00000002
#define FIEMAP_EXTENT_LAST 0x00000001
#define FIEMAP_EXTENT_UNKNOWN 0x00000002
#define FIEMAP_EXTENT_ENCODED 0x00000008
#define FIEMAP_EXTENT_DATA_ENCRYPTED 0x00000080
#define FIEMAP_EXTENT_NOT_ALIGNED 0x00000100
#define FIEMAP_EXTENT_DATA_INLINE 0x00000200
#define FIEMAP_EXTENT_MERGED 0x00001000
int fiemap_fill_next_extent(struct fiemap_extent_info *info, u64 logical, u64 phys, u64 len, u32 flags);
int fiemap_prep(struct inode *inode, struct fiemap_extent_info *fieinfo, u64 start, u64 *len, u32 supported_flags);
#endif

/* uKernel hamis <linux/vfs.h> — kstatfs (statfs). */
#ifndef _UK_LINUX_VFS_H
#define _UK_LINUX_VFS_H
#include <linux/types.h>
#ifndef _UK_KERNEL_FSID
#define _UK_KERNEL_FSID
typedef struct { int val[2]; } __kernel_fsid_t;
#endif
struct kstatfs {
	long f_type;
	long f_bsize;
	u64 f_blocks;
	u64 f_bfree;
	u64 f_bavail;
	u64 f_files;
	u64 f_ffree;
	__kernel_fsid_t f_fsid;
	long f_namelen;
	long f_frsize;
	long f_flags;
	long f_spare[4];
};
#endif

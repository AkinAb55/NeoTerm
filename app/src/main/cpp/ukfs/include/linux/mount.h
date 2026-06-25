/* uKernel hamis <linux/mount.h> — vfsmount stub. */
#ifndef _UK_LINUX_MOUNT_H
#define _UK_LINUX_MOUNT_H
#include <linux/types.h>
struct dentry;
struct super_block;
struct vfsmount {
	struct dentry *mnt_root;
	struct super_block *mnt_sb;
	int mnt_flags;
};
#define MNT_NOSUID 0x01
#define MNT_NODEV  0x02
#define MNT_NOEXEC 0x04
#endif

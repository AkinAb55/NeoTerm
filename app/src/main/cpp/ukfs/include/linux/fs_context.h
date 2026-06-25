/* uKernel hamis <linux/fs_context.h> — mount-kontextus felülete. */
#ifndef _UK_LINUX_FS_CONTEXT_H
#define _UK_LINUX_FS_CONTEXT_H
#include <linux/types.h>

struct super_block;
struct dentry;
struct file_system_type;
struct fs_parameter;
struct fs_parameter_spec;

enum fs_context_purpose {
	FS_CONTEXT_FOR_MOUNT,
	FS_CONTEXT_FOR_SUBMOUNT,
	FS_CONTEXT_FOR_RECONFIGURE,
};

struct fs_context {
	const struct fs_context_operations *ops;
	struct file_system_type *fs_type;
	void *fs_private;
	void *s_fs_info;
	struct super_block *root_sb;       /* shim */
	struct dentry *root;
	const char *source;
	unsigned int sb_flags;
	unsigned int sb_flags_mask;
	enum fs_context_purpose purpose;
	void *security;
};

struct fs_context_operations {
	void (*free)(struct fs_context *fc);
	int (*dup)(struct fs_context *fc, struct fs_context *src_fc);
	int (*parse_param)(struct fs_context *fc, struct fs_parameter *param);
	int (*parse_monolithic)(struct fs_context *fc, void *data);
	int (*get_tree)(struct fs_context *fc);
	int (*reconfigure)(struct fs_context *fc);
};

/* az F3-shim adja: a bdev-alapú superblock felépítése a fill_super callbackkel */
int get_tree_bdev(struct fs_context *fc,
		  int (*fill_super)(struct super_block *sb, struct fs_context *fc));
int get_tree_nodev(struct fs_context *fc,
		   int (*fill_super)(struct super_block *sb, struct fs_context *fc));

#define warnf(fc, fmt, ...)	do {} while (0)
#define errorf(fc, fmt, ...)	do {} while (0)
#define invalf(fc, fmt, ...)	(-22)
#endif

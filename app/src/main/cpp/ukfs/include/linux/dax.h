/* uKernel hamis <linux/dax.h> — stub. */
#ifndef _UK_LINUX_DAX_H
#define _UK_LINUX_DAX_H
#include <linux/types.h>
struct dax_device; struct block_device;
struct dax_holder_operations { void (*notify_failure)(struct dax_device *dax_dev, u64 off, u64 len, int mf_flags); };
struct dax_device *fs_dax_get_by_bdev(struct block_device *bdev, u64 *start_off, void *holder, const struct dax_holder_operations *ops);
#endif

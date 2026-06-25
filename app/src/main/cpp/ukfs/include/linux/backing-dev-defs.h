/* uKernel hamis <linux/backing-dev-defs.h> — backing_dev_info stub. */
#ifndef _UK_LINUX_BACKING_DEV_DEFS_H
#define _UK_LINUX_BACKING_DEV_DEFS_H
#include <linux/types.h>
struct backing_dev_info {
	unsigned long ra_pages;
	unsigned long io_pages;
	char *name;
};
#endif

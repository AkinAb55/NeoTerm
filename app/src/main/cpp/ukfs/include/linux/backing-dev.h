/* uKernel hamis <linux/backing-dev.h> — backing-dev felület stub. */
#ifndef _UK_LINUX_BACKING_DEV_H
#define _UK_LINUX_BACKING_DEV_H
#include <linux/backing-dev-defs.h>
struct super_block;
static inline unsigned long bdi_min_ratio(void) { return 0; }
#define BDI_CAP_NO_ACCT_DIRTY 0
#endif

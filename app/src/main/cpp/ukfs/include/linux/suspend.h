#ifndef _UK_LINUX_SUSPEND_H
#define _UK_LINUX_SUSPEND_H
#include <linux/types.h>
static inline int pm_suspend_via_firmware(void) { return 0; }
static inline int pm_resume_via_firmware(void) { return 0; }
#endif

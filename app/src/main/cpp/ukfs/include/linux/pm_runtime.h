/* uKernel hamis <linux/pm_runtime.h> — runtime PM no-op stubok. */
#ifndef _UK_LINUX_PM_RUNTIME_H
#define _UK_LINUX_PM_RUNTIME_H

#include <linux/device.h>

static inline int  pm_runtime_get(struct device *dev) { (void)dev; return 0; }
static inline int  pm_runtime_get_sync(struct device *dev) { (void)dev; return 0; }
static inline int  pm_runtime_get_noresume(struct device *dev) { (void)dev; return 0; }
static inline int  pm_runtime_put(struct device *dev) { (void)dev; return 0; }
static inline int  pm_runtime_put_sync(struct device *dev) { (void)dev; return 0; }
static inline int  pm_runtime_put_noidle(struct device *dev) { (void)dev; return 0; }
static inline void pm_runtime_enable(struct device *dev) { (void)dev; }
static inline void pm_runtime_disable(struct device *dev) { (void)dev; }
static inline void pm_runtime_set_active(struct device *dev) { (void)dev; }
static inline void pm_runtime_mark_last_busy(struct device *dev) { (void)dev; }
static inline void pm_runtime_use_autosuspend(struct device *dev) { (void)dev; }
static inline void pm_runtime_set_autosuspend_delay(struct device *dev, int d) { (void)dev; (void)d; }
static inline int  pm_runtime_set_suspended(struct device *dev) { (void)dev; return 0; }
static inline int  pm_runtime_resume(struct device *dev) { (void)dev; return 0; }
static inline int  pm_runtime_idle(struct device *dev) { (void)dev; return 0; }
static inline void pm_runtime_get_noresume_x(struct device *dev) { (void)dev; }

#endif

#ifndef _UK_LINUX_SCHED_MM_H
#define _UK_LINUX_SCHED_MM_H
#include <linux/types.h>
static inline unsigned int memalloc_nofs_save(void) { return 0; }
static inline void memalloc_nofs_restore(unsigned int flags) { (void)flags; }
static inline unsigned int memalloc_noreclaim_save(void) { return 0; }
static inline void memalloc_noreclaim_restore(unsigned int flags) { (void)flags; }
#endif

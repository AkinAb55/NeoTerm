#ifndef _UK_LINUX_BIT_SPINLOCK_H
#define _UK_LINUX_BIT_SPINLOCK_H
#include <linux/types.h>
static inline void bit_spin_lock(int bitnum, unsigned long *addr) { (void)bitnum;(void)addr; }
static inline void bit_spin_unlock(int bitnum, unsigned long *addr) { (void)bitnum;(void)addr; }
static inline int bit_spin_is_locked(int bitnum, unsigned long *addr) { (void)bitnum;(void)addr; return 0; }
static inline int bit_spin_trylock(int bitnum, unsigned long *addr) { (void)bitnum;(void)addr; return 1; }
#endif

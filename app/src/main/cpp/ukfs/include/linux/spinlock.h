/* uKernel hamis <linux/spinlock.h> — pthread mutex-re képezve.
 * Userspace-ben nincs IRQ-kontextus, így az _irqsave változatok csak a flags-et
 * formálisan kezelik. */
#ifndef _UK_LINUX_SPINLOCK_H
#define _UK_LINUX_SPINLOCK_H

#include <linux/types.h>
#include <pthread.h>

typedef struct { pthread_mutex_t m; int inited; } spinlock_t;
typedef spinlock_t rwlock_t;

#define __SPIN_LOCK_UNLOCKED { PTHREAD_MUTEX_INITIALIZER, 1 }
#define DEFINE_SPINLOCK(x) spinlock_t x = __SPIN_LOCK_UNLOCKED

void spin_lock_init(spinlock_t *l);
void spin_lock(spinlock_t *l);
void spin_unlock(spinlock_t *l);
/* IRQ-változatok: ugyanaz a mutex; a flags-et nullázzuk. */
#define spin_lock_irqsave(l, flags)   do { (flags) = 0; spin_lock(l); } while (0)
#define spin_unlock_irqrestore(l, fl) do { (void)(fl); spin_unlock(l); } while (0)
#define spin_lock_bh(l)               spin_lock(l)
#define spin_unlock_bh(l)             spin_unlock(l)
#define spin_lock_irq(l)              spin_lock(l)
#define spin_unlock_irq(l)            spin_unlock(l)

#define rwlock_init(l)        spin_lock_init(l)
#define read_lock(l)          spin_lock(l)
#define read_unlock(l)        spin_unlock(l)
#define write_lock(l)         spin_lock(l)
#define write_unlock(l)       spin_unlock(l)
#define read_lock_bh(l)       spin_lock(l)
#define read_unlock_bh(l)     spin_unlock(l)
#define write_lock_bh(l)      spin_lock(l)
#define write_unlock_bh(l)    spin_unlock(l)

#define spin_lock_nested(l, subclass)        spin_lock(l)
#define spin_lock_bh_nested(l, subclass)     spin_lock(l)
#define spin_lock_irqsave_nested(l, fl, sc)  spin_lock_irqsave(l, fl)

#define local_bh_disable()    do {} while (0)
#define local_bh_enable()     do {} while (0)
#define local_irq_save(f)     do { (f) = 0; } while (0)
#define local_irq_restore(f)  do { (void)(f); } while (0)

/* lockdep — a userspace shimben nincs lock-validator, no-op. */
#ifndef lockdep_assert_held
#define lockdep_assert_held(l)		do { (void)(l); } while (0)
#define lockdep_assert_held_once(l)	do { (void)(l); } while (0)
#define lockdep_assert_not_held(l)	do { (void)(l); } while (0)
#endif

#endif

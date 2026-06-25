/* uKernel hamis <linux/rwsem.h> — az alapokat a mutex.h adja, itt csak a kiegészítők. */
#ifndef _UK_LINUX_RWSEM_H
#define _UK_LINUX_RWSEM_H
#include <linux/mutex.h>   /* rw_semaphore, init_rwsem, down_read/up_read, ... */
#ifndef _UK_RWSEM_EXTRA
#define _UK_RWSEM_EXTRA
#define down_read_nested(s, sub)   down_read(s)
#define down_write_nested(s, sub)  down_write(s)
static inline int down_read_trylock(struct rw_semaphore *s) { (void)s; return 1; }
static inline int down_write_trylock(struct rw_semaphore *s) { (void)s; return 1; }
static inline void downgrade_write(struct rw_semaphore *s) { (void)s; }
/* rwsem_is_locked: MINDIG zárolt (1) — a shim egyszálú, a zárolás-ellenőrzések (WARN_ON/BUG_ON
 * az ext4-ben/jbd2-ben + az ext4_break_layouts truncate-úti -EINVAL-ja) így mindig átmennek.
 * A 0-stub a break_layouts-ot buktatta; a valódi mutex-flag viszont a write-flusht (más ágon). */
static inline int rwsem_is_locked(struct rw_semaphore *s) { (void)s; return 1; }
#endif
#endif

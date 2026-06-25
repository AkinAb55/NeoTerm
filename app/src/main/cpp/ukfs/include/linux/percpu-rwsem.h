#ifndef _UK_LINUX_PERCPU_RWSEM_H
#define _UK_LINUX_PERCPU_RWSEM_H
#include <linux/rwsem.h>
struct percpu_rw_semaphore { struct rw_semaphore rss; };
static inline void percpu_down_read(struct percpu_rw_semaphore *s) { (void)s; }
static inline void percpu_up_read(struct percpu_rw_semaphore *s) { (void)s; }
static inline void percpu_down_write(struct percpu_rw_semaphore *s) { (void)s; }
static inline void percpu_up_write(struct percpu_rw_semaphore *s) { (void)s; }
static inline int percpu_init_rwsem(struct percpu_rw_semaphore *s) { (void)s; return 0; }
static inline void percpu_free_rwsem(struct percpu_rw_semaphore *s) { (void)s; }
#define init_rwsem_percpu percpu_init_rwsem
#endif

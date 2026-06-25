#ifndef _UK_LINUX_PERCPU_COUNTER_H
#define _UK_LINUX_PERCPU_COUNTER_H
#include <linux/types.h>
#include <linux/spinlock.h>
struct percpu_counter { s64 count; spinlock_t lock; int uk_init; };
static inline int percpu_counter_init(struct percpu_counter *c, s64 amount, gfp_t gfp) { (void)gfp; c->count = amount; c->uk_init = 1; return 0; }
/* percpu_counter_initialized: VALÓDI állapot (init flag) — különben a mount-idejű
 * commit_super a még-nem-inicializált (0) számlálót olvasná és NULLÁZNÁ az es-t! */
static inline int percpu_counter_initialized(struct percpu_counter *c) { return c && c->uk_init; }
static inline void percpu_counter_destroy(struct percpu_counter *c) { (void)c; }
static inline void percpu_counter_set(struct percpu_counter *c, s64 amount) { c->count = amount; }
static inline void percpu_counter_add(struct percpu_counter *c, s64 amount) { c->count += amount; }
static inline void percpu_counter_sub(struct percpu_counter *c, s64 amount) { c->count -= amount; }
static inline s64 percpu_counter_sum(struct percpu_counter *c) { return c->count; }
static inline s64 percpu_counter_read(struct percpu_counter *c) { return c->count; }
static inline s64 percpu_counter_read_positive(struct percpu_counter *c) { return c->count > 0 ? c->count : 0; }
#define percpu_counter_add_batch(c, a, b) percpu_counter_add((c), (a))
static inline int percpu_counter_compare(struct percpu_counter *c, s64 rhs) { return c->count < rhs ? -1 : c->count > rhs ? 1 : 0; }
static inline s64 percpu_counter_sum_positive(struct percpu_counter *c) { return c->count > 0 ? c->count : 0; }
static inline void percpu_counter_inc(struct percpu_counter *c) { c->count++; }
static inline void percpu_counter_dec(struct percpu_counter *c) { c->count--; }
#endif

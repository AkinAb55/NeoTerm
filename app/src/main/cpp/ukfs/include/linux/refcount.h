#ifndef _UK_LINUX_REFCOUNT_H
#define _UK_LINUX_REFCOUNT_H
#include <linux/atomic.h>
typedef struct { atomic_t refs; } refcount_t;
static inline void refcount_set(refcount_t *r, int n) { r->refs.counter = n; }
static inline unsigned int refcount_read(const refcount_t *r) { return r->refs.counter; }
static inline void refcount_inc(refcount_t *r) { r->refs.counter++; }
static inline bool refcount_dec_and_test(refcount_t *r) { return --r->refs.counter == 0; }
static inline bool refcount_inc_not_zero(refcount_t *r) { if (r->refs.counter) { r->refs.counter++; return true; } return false; }
#define REFCOUNT_INIT(n) { .refs = { (n) } }
#endif

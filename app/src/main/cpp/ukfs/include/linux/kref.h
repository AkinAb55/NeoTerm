#ifndef _UK_LINUX_KREF_H
#define _UK_LINUX_KREF_H
#include <linux/atomic.h>
struct kref { atomic_t refcount; };
static inline void kref_init(struct kref *k){ atomic_set(&k->refcount,1); }
static inline void kref_get(struct kref *k){ atomic_inc(&k->refcount); }
static inline int kref_put(struct kref *k, void (*rel)(struct kref *)){ if(atomic_dec_and_test(&k->refcount)){ if(rel)rel(k); return 1;} return 0; }
#endif

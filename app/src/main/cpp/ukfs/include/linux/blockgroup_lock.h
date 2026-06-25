#ifndef _UK_LINUX_BLOCKGROUP_LOCK_H
#define _UK_LINUX_BLOCKGROUP_LOCK_H
#include <linux/spinlock.h>
struct bgl_lock { spinlock_t lock; };
struct blockgroup_lock { struct bgl_lock locks[1]; };
static inline void bgl_lock_init(struct blockgroup_lock *bgl) { (void)bgl; }
static inline spinlock_t *bgl_lock_ptr(struct blockgroup_lock *bgl, unsigned int block_group) { (void)block_group; return &bgl->locks[0].lock; }
#define NR_BG_LOCKS 1
#endif

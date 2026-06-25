#ifndef _UK_LINUX_XARRAY_H
#define _UK_LINUX_XARRAY_H
#include <linux/types.h>
#include <linux/spinlock.h>
typedef unsigned __bitwise xa_mark_t;
struct xarray { spinlock_t xa_lock; unsigned int xa_flags; void *xa_head; };
#define XA_FLAGS_LOCK_IRQ 0
#define XARRAY_INIT(name, flags) { .xa_flags = (flags) }
#define DEFINE_XARRAY(name) struct xarray name
#define xa_for_each(xa, index, entry) for ((index)=0, (entry)=NULL; 0; )
#define xa_for_each_range(xa, index, entry, start, last) for ((index)=(start), (entry)=NULL; 0; )
#define xa_for_each_start(xa, index, entry, start) for ((index)=(start), (entry)=NULL; 0; )
#define xa_for_each_marked(xa, index, entry, filter) for ((index)=0, (entry)=NULL; 0; )
static inline void xa_init(struct xarray *xa) { xa->xa_head = NULL; }
static inline void xa_init_flags(struct xarray *xa, unsigned int flags) { xa->xa_head = NULL; xa->xa_flags = flags; }
static inline bool xa_empty(const struct xarray *xa) { return xa->xa_head == NULL; }
void *xa_load(struct xarray *xa, unsigned long index);
void *xa_store(struct xarray *xa, unsigned long index, void *entry, gfp_t gfp);
void *xa_erase(struct xarray *xa, unsigned long index);
void xa_destroy(struct xarray *xa);
int xa_insert(struct xarray *xa, unsigned long index, void *entry, gfp_t gfp);
static inline bool xa_is_err(const void *entry) { (void)entry; return false; }
#endif

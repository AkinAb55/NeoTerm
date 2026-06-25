#ifndef _UK_LINUX_PERCPU_H
#define _UK_LINUX_PERCPU_H
#include <linux/percpu-defs.h>
#include <linux/slab.h>
#define alloc_percpu(type) kzalloc(sizeof(type), GFP_KERNEL)
#define alloc_percpu_gfp(type, gfp) kzalloc(sizeof(type), (gfp))
#define free_percpu(ptr) kfree(ptr)
#endif

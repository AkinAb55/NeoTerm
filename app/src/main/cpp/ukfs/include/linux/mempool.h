#ifndef _UK_LINUX_MEMPOOL_H
#define _UK_LINUX_MEMPOOL_H
#include <linux/types.h>
typedef struct mempool_s mempool_t;
typedef void *(*mempool_alloc_t)(gfp_t, void *);
typedef void (*mempool_free_t)(void *, void *);
mempool_t *mempool_create(int min_nr, mempool_alloc_t alloc_fn, mempool_free_t free_fn, void *pool_data);
void mempool_destroy(mempool_t *pool);
void *mempool_alloc(mempool_t *pool, gfp_t gfp);
void mempool_free(void *element, mempool_t *pool);
void *mempool_alloc_slab(gfp_t gfp, void *pool_data);
void mempool_free_slab(void *element, void *pool_data);
#define mempool_create_slab_pool(_min_nr, _kc) mempool_create((_min_nr), mempool_alloc_slab, mempool_free_slab, (void *)(_kc))
#endif

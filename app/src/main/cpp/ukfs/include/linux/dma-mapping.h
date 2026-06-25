#ifndef _UK_LINUX_DMA_MAPPING_H
#define _UK_LINUX_DMA_MAPPING_H
#include <linux/types.h>
#include <linux/slab.h>
#define DMA_BIT_MASK(n) (((n) == 64) ? ~0ULL : ((1ULL << (n)) - 1))

#ifndef _UK_DMA_DATA_DIRECTION
#define _UK_DMA_DATA_DIRECTION
enum dma_data_direction {
	DMA_BIDIRECTIONAL = 0,
	DMA_TO_DEVICE = 1,
	DMA_FROM_DEVICE = 2,
	DMA_NONE = 3,
};
#endif
static inline void *dma_alloc_coherent(struct device *d, size_t s, dma_addr_t *h, gfp_t f)
{ (void)d; (void)f; void *p = kzalloc(s, 0); if (h) *h = (dma_addr_t)(unsigned long)p; return p; }
static inline void dma_free_coherent(struct device *d, size_t s, void *v, dma_addr_t h)
{ (void)d; (void)s; (void)h; kfree(v); }
static inline int dma_set_mask(struct device *d, u64 m) { (void)d; (void)m; return 0; }
static inline int dma_set_coherent_mask(struct device *d, u64 m) { (void)d; (void)m; return 0; }
#endif

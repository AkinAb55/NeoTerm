/* uKernel hamis <linux/scatterlist.h> — minimalis scatter-gather (usbnet SG TX). */
#ifndef _UK_LINUX_SCATTERLIST_H
#define _UK_LINUX_SCATTERLIST_H

#include <linux/types.h>
#include <linux/string.h>

struct scatterlist {
	unsigned long	page_link;
	unsigned int	offset;
	unsigned int	length;
	dma_addr_t	dma_address;
	void		*buf;
};

static inline void sg_init_table(struct scatterlist *sgl, unsigned int nents)
{ memset(sgl, 0, sizeof(*sgl) * nents); }

static inline void sg_init_one(struct scatterlist *sg, const void *buf, unsigned int buflen)
{ memset(sg, 0, sizeof(*sg)); sg->buf = (void *)buf; sg->length = buflen; }

static inline void sg_set_buf(struct scatterlist *sg, const void *buf, unsigned int buflen)
{ sg->buf = (void *)buf; sg->length = buflen; }

static inline void sg_set_page(struct scatterlist *sg, void *page, unsigned int len, unsigned int offset)
{ sg->buf = page; sg->length = len; sg->offset = offset; }

static inline void *sg_virt(struct scatterlist *sg) { return sg->buf; }
static inline void sg_mark_end(struct scatterlist *sg) { (void)sg; }

#define for_each_sg(sglist, sg, nr, __i) \
	for (__i = 0, sg = (sglist); __i < (nr); __i++, sg++)

/* sg_table — a (sgl, nents) par, ahogy a scsi_data_buffer hasznalja. */
struct sg_table {
	struct scatterlist	*sgl;
	unsigned int		nents;
	unsigned int		orig_nents;
};

static inline struct scatterlist *sg_next(struct scatterlist *sg)
{ return sg ? (sg + 1) : NULL; }

static inline int sg_nents(struct scatterlist *sg)
{ int n = 0; while (sg && sg->length) { n++; sg++; } return n ? n : 1; }

/* sg_mapping_iter — a flat-buffer SG-modellben az "addr" egyszeruen sg->buf. */
#define SG_MITER_ATOMIC		(1 << 0)
#define SG_MITER_TO_SG		(1 << 1)
#define SG_MITER_FROM_SG	(1 << 2)

struct sg_page_iter {
	struct scatterlist	*sg;
	unsigned int		sg_pgoffset;
};

struct sg_mapping_iter {
	void			*addr;
	size_t			length;
	size_t			consumed;
	struct sg_page_iter	piter;
	/* belso allapot */
	unsigned int		__nents;
	unsigned int		__idx;
	unsigned int		__flags;
	struct scatterlist	*__sgl;
	size_t			__sg_off;
};

static inline void sg_miter_start(struct sg_mapping_iter *miter,
		struct scatterlist *sgl, unsigned int nents, unsigned int flags)
{
	memset(miter, 0, sizeof(*miter));
	miter->__sgl = sgl;
	miter->__nents = nents;
	miter->__flags = flags;
	miter->piter.sg = sgl;
}

static inline bool sg_miter_skip(struct sg_mapping_iter *miter, off_t offset)
{
	while (offset > 0 && miter->__idx < miter->__nents) {
		struct scatterlist *sg = &miter->__sgl[miter->__idx];
		size_t avail = sg->length - miter->__sg_off;
		if ((size_t)offset < avail) {
			miter->__sg_off += offset;
			return true;
		}
		offset -= avail;
		miter->__idx++;
		miter->__sg_off = 0;
	}
	return offset == 0;
}

static inline bool sg_miter_next(struct sg_mapping_iter *miter)
{
	struct scatterlist *sg;
	if (miter->__idx >= miter->__nents)
		return false;
	sg = &miter->__sgl[miter->__idx];
	miter->piter.sg = sg;
	miter->addr = (char *)sg->buf + miter->__sg_off;
	miter->length = sg->length - miter->__sg_off;
	miter->__idx++;
	miter->__sg_off = 0;
	return true;
}

static inline void sg_miter_stop(struct sg_mapping_iter *miter) { (void)miter; }

#endif

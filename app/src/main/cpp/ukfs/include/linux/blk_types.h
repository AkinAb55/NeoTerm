/* uKernel hamis <linux/blk_types.h>. */
#ifndef _UK_LINUX_BLK_TYPES_H
#define _UK_LINUX_BLK_TYPES_H
#include <linux/types.h>
typedef u8 blk_status_t;
typedef unsigned int blk_opf_t;
struct bio_vec { void *bv_page; unsigned bv_len; unsigned bv_offset; };
struct bvec_iter { sector_t bi_sector; unsigned bi_size; unsigned bi_idx; unsigned bi_bvec_done; };
struct block_device;
struct bio { struct block_device *bi_bdev; blk_status_t bi_status; blk_opf_t bi_opf; struct bvec_iter bi_iter; void (*bi_end_io)(struct bio *); void *bi_private; unsigned short bi_vcnt; unsigned short bi_write_hint; struct bio_vec *bi_io_vec; };
static inline int blk_status_to_errno(blk_status_t s){ return s ? -5 : 0; }
struct block_device;
#define REQ_OP_READ	0
#define REQ_OP_WRITE	1
#define REQ_SYNC	(1 << 3)
#define REQ_META	(1 << 4)
#define REQ_FUA		(1 << 9)
#define REQ_PREFLUSH	(1 << 10)
#define REQ_RAHEAD	(1 << 11)
#define REQ_IDLE	(1 << 12)
#define REQ_PRIO	(1 << 5)
#define REQ_RAHEAD	(1 << 6)
#define BLK_STS_OK	0
#define BIO_MAX_VECS 256
struct folio; struct gfp;
struct folio_iter { struct folio *folio; size_t offset; size_t length; size_t _seg_count; int _i; };
struct bio *bio_alloc(struct block_device *bdev, unsigned short nr, blk_opf_t opf, gfp_t gfp);
void bio_put(struct bio *bio);
void submit_bio(struct bio *bio);
int submit_bio_wait(struct bio *bio);
void bio_add_folio_nofail(struct bio *bio, struct folio *folio, size_t len, size_t off);
#define bio_for_each_folio_all(fi, bio) for ((fi).folio = NULL; 0; )
#define BLK_STS_IOERR	10
#endif

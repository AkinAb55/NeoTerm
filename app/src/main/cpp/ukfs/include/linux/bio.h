/* uKernel hamis <linux/bio.h> — a struct bio a blk_types.h-ban; itt a műveletek. */
#ifndef _UK_LINUX_BIO_H
#define _UK_LINUX_BIO_H
#include <linux/blk_types.h>
#include <linux/blkdev.h>
struct bio *bio_alloc(struct block_device *bdev, unsigned short nr, blk_opf_t opf, gfp_t gfp);
void bio_put(struct bio *bio);
void submit_bio(struct bio *bio);
int submit_bio_wait(struct bio *bio);
void bio_add_folio_nofail(struct bio *bio, struct folio *folio, size_t len, size_t off);
#define BIO_MAX_VECS 256
#endif

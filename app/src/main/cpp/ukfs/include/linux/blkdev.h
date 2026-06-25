/* uKernel hamis <linux/blkdev.h> — a usb-storage minimalis blokk-reteg igenye:
 * struct request_queue elore-deklaracio, SECTOR_* es par alap-tipus/limit. */
#ifndef _UK_LINUX_BLKDEV_H
#define _UK_LINUX_BLKDEV_H

#include <linux/types.h>
#include <linux/string.h>
#include <linux/blk_types.h>
#define BDEVNAME_SIZE 32
typedef unsigned int blk_mode_t;
struct io_comp_batch;
struct blk_holder_ops { void (*mark_dead)(struct block_device *bdev, bool surprise); void (*sync)(struct block_device *bdev); int (*freeze)(struct block_device *bdev); int (*thaw)(struct block_device *bdev); };
extern const struct blk_holder_ops fs_holder_ops;
int iocb_bio_iopoll(struct kiocb *kiocb, struct io_comp_batch *iob, unsigned int flags);
struct file *bdev_file_open_by_dev(dev_t dev, blk_mode_t mode, void *holder, const struct blk_holder_ops *hops);
#define BLK_OPEN_READ (1u<<0)
#define BLK_OPEN_WRITE (1u<<1)
#define BLK_OPEN_RESTRICT_WRITES (1u<<5)

/* blk_plug — a fat fatent.c blokk-elő-ütemezője (no-op a shimben) */
struct blk_plug { int dummy; };
static inline void blk_start_plug(struct blk_plug *plug) { (void)plug; }
static inline void blk_finish_plug(struct blk_plug *plug) { (void)plug; }


/* blokk-kérés flagek (exfat/ext4 sync) */
#ifndef REQ_SYNC
#define REQ_SYNC	(1 << 3)
#define REQ_FUA		(1 << 9)
#define REQ_PREFLUSH	(1 << 10)
#define REQ_META	(1 << 4)
#define REQ_OP_READ	0
#define REQ_OP_WRITE	1
#endif

#ifndef SECTOR_SHIFT
#define SECTOR_SHIFT	9
#endif
#ifndef SECTOR_SIZE
#define SECTOR_SIZE	(1 << SECTOR_SHIFT)
#endif

struct bio;

struct gendisk {
	char	disk_name[32];
	void	*private_data;
};

/* A usb-storage transport.c a srb->request->q->disk lancot dereferalja a
 * kapacitas-hack ellenorzesehez; ezek minimalis valodi tipusok. */
struct request_queue {
	struct gendisk	*disk;
};

struct request {
	struct request_queue	*q;
};

/* queue_limits — a scsi sdev_configure(lim) altal hasznalt mezok. */
struct queue_limits {
	unsigned int		max_hw_sectors;
	unsigned int		max_sectors;
	unsigned int		max_segment_size;
	unsigned int		dma_alignment;
};

typedef unsigned int blk_qc_t;
typedef u32 blk_opf_t;
typedef unsigned int blk_mq_req_flags_t;

/* blk_status_t — a kernel sd.h inline-jai hivatkozzak. */
typedef u8 blk_status_t;
#define BLK_STS_OK	0
#define BLK_STS_TARGET	((blk_status_t)4)
#define BLK_STS_NOTSUPP	((blk_status_t)1)

/* queue_limits update API — a usb-storage write_info()-ja hasznalja. */
static inline struct queue_limits
queue_limits_start_update(struct request_queue *q)
{ struct queue_limits lim; (void)q; memset(&lim, 0, sizeof(lim)); return lim; }
static inline int
queue_limits_commit_update_frozen(struct request_queue *q, struct queue_limits *lim)
{ (void)q; (void)lim; return 0; }

#define SG_MAX_SEGMENTS	128
#define SG_ALL		SG_MAX_SEGMENTS
#define SG_CHUNK_SIZE	128

static inline unsigned int queue_max_hw_sectors(struct request_queue *q)
{ (void)q; return 0; }

#endif

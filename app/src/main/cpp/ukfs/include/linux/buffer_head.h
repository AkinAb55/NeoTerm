/* uKernel hamis <linux/buffer_head.h> — blokk-puffer (buffer_head) felülete.
 * Az F3-shim a sb_bread-et a libukblk blokk-rétegre (BOT) köti. */
#ifndef _UK_LINUX_BUFFER_HEAD_H
#define _UK_LINUX_BUFFER_HEAD_H
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/blk_types.h>
#include <linux/spinlock.h>     /* struct super_block/inode — a fat.h innen kapja a VFS-t */

#define MAX_BUF_PER_PAGE	(PAGE_SIZE / 512)

enum bh_state_bits {
	BH_Uptodate, BH_Dirty, BH_Locked, BH_Req, BH_Mapped, BH_New,
	BH_Async_Read, BH_Async_Write, BH_Delay, BH_Boundary, BH_Write_EIO,
	BH_Unwritten, BH_Quiet, BH_Meta, BH_Prio, BH_Defer_Completion,
	BH_PrivateStart,   /* a jbd2 innen foglal saját biteket (BH_JBD stb.) — BH_Verified is itt! */
};

struct buffer_head {
	unsigned long b_state;
	struct buffer_head *b_this_page;
	void *b_page;
	sector_t b_blocknr;
	size_t b_size;
	char *b_data;
	struct folio *b_folio;
	void *b_bdev;
	struct super_block *b_bdev_sb;   /* shim: melyik sb-hez tartozik */
	void (*b_end_io)(struct buffer_head *bh, int uptodate);
	void *b_private;
	int b_count;
	struct list_head b_assoc_buffers;
	spinlock_t b_uptodate_lock;
};

/* állapot-bitek */
#define BUFFER_FNS(bit, name)							\
static inline void set_buffer_##name(struct buffer_head *bh) { bh->b_state |= (1UL << BH_##bit); }	\
static inline void clear_buffer_##name(struct buffer_head *bh) { bh->b_state &= ~(1UL << BH_##bit); }	\
static inline int buffer_##name(const struct buffer_head *bh) { return (bh->b_state >> BH_##bit) & 1; }
/* test-and-set/clear (a jbd2 használja) */
#define TAS_BUFFER_FNS(bit, name)						\
static inline int test_set_buffer_##name(struct buffer_head *bh) { int o = (bh->b_state >> BH_##bit) & 1; bh->b_state |= (1UL << BH_##bit); return o; }	\
static inline int test_clear_buffer_##name(struct buffer_head *bh) { int o = (bh->b_state >> BH_##bit) & 1; bh->b_state &= ~(1UL << BH_##bit); return o; }
BUFFER_FNS(Uptodate, uptodate)
BUFFER_FNS(Dirty, dirty)
BUFFER_FNS(Locked, locked)
BUFFER_FNS(Mapped, mapped)
BUFFER_FNS(New, new)
BUFFER_FNS(Async_Read, async_read)
BUFFER_FNS(Async_Write, async_write)
BUFFER_FNS(Delay, delay)
BUFFER_FNS(Boundary, boundary)
BUFFER_FNS(Unwritten, unwritten)
BUFFER_FNS(Req, req)
BUFFER_FNS(Meta, meta)

/* az F3-shim implementálja ezeket (BOT-on át) */
/* get_block_t a kernelben FÜGGVÉNY-típus (nem pointer); a prototípusok get_block_t* alakot kérnek */
typedef void (bh_end_io_t)(struct buffer_head *bh, int uptodate);
typedef int (get_block_t)(struct inode *inode, sector_t iblock, struct buffer_head *bh_result, int create);
struct folio;
struct writeback_control;
struct address_space;
struct kiocb;
int block_read_full_folio(struct folio *, get_block_t *);
int block_write_full_folio(struct folio *folio, struct writeback_control *wbc, void *get_block);
int block_write_begin(struct address_space *mapping, loff_t pos, unsigned len, struct folio **foliop, get_block_t *get_block);
int block_write_end(loff_t pos, unsigned len, unsigned copied, struct folio *);
int generic_write_end(const struct kiocb *, struct address_space *, loff_t, unsigned len, unsigned copied, struct folio *, void *);
int cont_write_begin(const struct kiocb *, struct address_space *, loff_t, unsigned, struct folio **, void **, get_block_t *, loff_t *);
bool block_dirty_folio(struct address_space *mapping, struct folio *folio);
int buffer_migrate_folio(struct address_space *, struct folio *dst, struct folio *src, int mode);
int buffer_migrate_folio_norefs(struct address_space *, struct folio *dst, struct folio *src, int mode);
int sync_mapping_buffers(struct address_space *mapping);
void block_invalidate_folio(struct folio *folio, size_t offset, size_t length);
sector_t generic_block_bmap(struct address_space *, sector_t, get_block_t *);

struct buffer_head *sb_bread(struct super_block *sb, sector_t block);
struct buffer_head *sb_bread_unmovable(struct super_block *sb, sector_t block);
struct buffer_head *sb_getblk(struct super_block *sb, sector_t block);
struct buffer_head *sb_find_get_block(struct super_block *sb, sector_t block);
struct buffer_head *sb_find_get_block_nonatomic(struct super_block *sb, sector_t block);
void sb_breadahead(struct super_block *sb, sector_t block);
struct buffer_head *__getblk(struct block_device *bdev, sector_t block, unsigned size);
struct buffer_head *__bread(struct super_block *sb, sector_t block, unsigned size);
struct buffer_head *__find_get_block_nonatomic(struct block_device *bdev, sector_t block, unsigned size);
struct buffer_head *__find_get_block(struct block_device *bdev, sector_t block, unsigned size);
struct buffer_head *alloc_buffer_head(gfp_t gfp_flags);
void free_buffer_head(struct buffer_head *bh);
struct buffer_head *bdev_getblk(struct block_device *bdev, sector_t block, unsigned size, gfp_t gfp);
struct buffer_head *getblk_unmovable(struct block_device *bdev, sector_t block, unsigned size);

void set_bh_page(struct buffer_head *bh, struct page *page, unsigned long offset);
void folio_set_bh(struct buffer_head *bh, struct folio *folio, unsigned long offset);
void brelse(struct buffer_head *bh);
void bforget(struct buffer_head *bh);
void __brelse(struct buffer_head *bh);
void mark_buffer_dirty(struct buffer_head *bh);
void mark_buffer_dirty_inode(struct buffer_head *bh, void *inode);
int sync_dirty_buffer(struct buffer_head *bh);
int __sync_dirty_buffer(struct buffer_head *bh, int op_flags);
void wait_on_buffer(struct buffer_head *bh);
void ll_rw_block(int op, int flags, int nr, struct buffer_head *bhs[]);
int submit_bh(blk_opf_t opf, struct buffer_head *bh);
void lock_buffer(struct buffer_head *bh);
void end_buffer_read_sync(struct buffer_head *bh, int uptodate);
void end_buffer_write_sync(struct buffer_head *bh, int uptodate);
int bh_uptodate_or_lock(struct buffer_head *bh);
int __bh_read(struct buffer_head *bh, blk_opf_t op_flags, bool wait);
void __bh_read_batch(int nr, struct buffer_head *bhs[], blk_opf_t op_flags, bool force_lock);
void bh_readahead(struct buffer_head *bh, blk_opf_t op_flags);
int sync_dirty_buffer(struct buffer_head *bh);
void write_boundary_block(struct block_device *bdev, sector_t bblock, unsigned blocksize);
void clean_bdev_aliases(struct block_device *bdev, sector_t block, sector_t len);
void folio_create_empty_buffers(struct folio *folio, unsigned long blocksize, unsigned long b_state);
struct buffer_head *create_empty_buffers(struct folio *folio, unsigned long blocksize, unsigned long b_state);
void unlock_buffer(struct buffer_head *bh);

static inline void get_bh(struct buffer_head *bh) { bh->b_count++; }
/* put_bh: NEM inline — az ntfs3 a buffereit (MFT-rekord, INDX-blokk) nb_put→put_bh-val
 * engedi el (nem brelse-szel). A kernelben a writeback a háttér-flush/umount dolga; a
 * shimnek nincs flush-szála, ezért a put_bh-nak vissza KELL írnia a piszkos buffert,
 * különben a módosított index-blokk sosem perzisztál. Lásd shim/fs/vfs.c. */
void put_bh(struct buffer_head *bh);
#endif

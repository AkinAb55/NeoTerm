/* uKernel hamis <linux/iomap.h> — iomap stub (ntfs3 read-úton minimál). */
#ifndef _UK_LINUX_IOMAP_H
#define _UK_LINUX_IOMAP_H
#include <linux/types.h>
struct iomap { u64 addr; loff_t offset; u64 length; u16 type; u16 flags; struct block_device *bdev; void *dax_dev; void *inline_data; void *private; const void *folio_ops; u64 validity_cookie; };
struct iomap_ops {
	int (*iomap_begin)(struct inode *inode, loff_t pos, loff_t length, unsigned flags, struct iomap *iomap, struct iomap *srcmap);
	int (*iomap_end)(struct inode *inode, loff_t pos, loff_t length, ssize_t written, unsigned flags, struct iomap *iomap);
};
struct iomap_iter { int dummy; };
#define IOMAP_HOLE	0
#define IOMAP_MAPPED	2
#define IOMAP_READ	0
#define IOMAP_NULL_ADDR ((u64)-1ULL)
#define IOMAP_REPORT 2
#define IOMAP_F_NEW 1
#define IOMAP_F_DIRTY 2
#define IOMAP_F_SHARED 4
#define IOMAP_F_BUFFER_HEAD 16
#define IOMAP_F_SIZE_CHANGED 32
#define IOMAP_F_BOUNDARY 64
#define IOMAP_F_XATTR 128
#define IOMAP_F_PRIVATE 0x1000
#define IOMAP_ZERO 16
#define IOMAP_NOWAIT 32
#define IOMAP_ATOMIC 256
#define IOMAP_FAULT 8
#define IOMAP_F_MERGED 4
#define IOMAP_DELALLOC 1
#define IOMAP_UNWRITTEN 4
#define IOMAP_INLINE 5
#define IOMAP_DIO_FORCE_WAIT (1 << 0)
#define IOMAP_WRITE 1
#define IOMAP_DIRECT 4
struct iomap_dio_ops { int (*end_io)(struct kiocb *iocb, ssize_t size, int error, unsigned flags); void (*submit_io)(const struct iomap_iter *iter, struct bio *bio, loff_t file_offset); void *bio_set; }; ssize_t iomap_dio_rw(struct kiocb *iocb, struct iov_iter *iter, const struct iomap_ops *ops, const struct iomap_dio_ops *dops, unsigned int flags, void *priv, size_t done);

struct readahead_control;
struct iomap_read_ops;
struct iomap_read_folio_ctx {
	const struct iomap_read_ops *ops;
	struct folio *cur_folio;
	struct readahead_control *rac;
	void *read_ctx;
	loff_t read_ctx_file_offset;
};

struct iomap_read_ops {
	int (*read_folio_range)(const struct iomap_iter *iter, void *ctx, size_t len);
	void (*submit_read)(const struct iomap_iter *iter, struct iomap_read_folio_ctx *ctx);
};
int iomap_bio_read_folio_range(const struct iomap_iter *iter, void *ctx, size_t len);

static inline void *iomap_inline_data(const struct iomap *iomap, loff_t pos) { return (char *)iomap->inline_data + pos - iomap->offset; }
struct iomap_writepage_ctx { struct iomap iomap; struct inode *inode; struct writeback_control *wbc; void *wb_ctx; const void *ops; u32 nr_folios; };
struct iomap_writeback_ops {
	int (*writeback_range)(struct iomap_writepage_ctx *wpc, struct folio *folio, u64 pos, unsigned len, u64 end_pos);
	int (*writeback_submit)(struct iomap_writepage_ctx *wpc, int error);
};
int iomap_ioend_writeback_submit(struct iomap_writepage_ctx *wpc, int error);
int iomap_writepages(struct iomap_writepage_ctx *wpc);

struct address_space; struct kiocb; struct iov_iter; struct writeback_control;
bool iomap_dirty_folio(struct address_space *mapping, struct folio *folio);
struct iomap_read_folio_ctx; void iomap_read_folio(const struct iomap_ops *ops, struct iomap_read_folio_ctx *ctx, void *p);
void iomap_readahead(const struct iomap_ops *ops, struct iomap_read_folio_ctx *ctx, void *priv);
sector_t iomap_bmap(struct address_space *mapping, sector_t bno, const struct iomap_ops *ops);
bool iomap_release_folio(struct folio *folio, gfp_t gfp);
void iomap_invalidate_folio(struct folio *folio, size_t offset, size_t len);
struct iomap_write_ops; ssize_t iomap_file_buffered_write(struct kiocb *iocb, struct iov_iter *from, const struct iomap_ops *ops, const struct iomap_write_ops *wops, void *priv);
int iomap_fiemap(struct inode *inode, struct fiemap_extent_info *fi, u64 start, u64 len, const struct iomap_ops *ops);
loff_t iomap_seek_hole(struct inode *inode, loff_t pos, const struct iomap_ops *ops);
loff_t iomap_seek_data(struct inode *inode, loff_t pos, const struct iomap_ops *ops);
int iomap_truncate_page(struct inode *inode, loff_t pos, bool *did_zero, const struct iomap_ops *ops);
int iomap_zero_range(struct inode *inode, loff_t pos, loff_t len, bool *did_zero, const struct iomap_ops *ops, const struct iomap_write_ops *wops, void *priv);

struct iomap_folio_ops {
	struct folio *(*get_folio)(struct iomap_iter *iter, loff_t pos, unsigned len);
	void (*put_folio)(struct inode *inode, loff_t pos, unsigned copied, struct folio *folio);
	bool (*iomap_valid)(struct inode *inode, const struct iomap *iomap);
};

struct iomap_write_ops {
	struct folio *(*get_folio)(struct iomap_iter *iter, loff_t pos, unsigned len);
	void (*put_folio)(struct inode *inode, loff_t pos, unsigned copied, struct folio *folio);
	bool (*iomap_valid)(struct inode *inode, const struct iomap *iomap);
	int (*read_folio_range)(const struct iomap_iter *iter, struct folio *folio, loff_t pos, size_t len);
};
#endif

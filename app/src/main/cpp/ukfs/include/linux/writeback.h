/* uKernel hamis <linux/writeback.h> — writeback_control stub. */
#ifndef _UK_LINUX_WRITEBACK_H
#define _UK_LINUX_WRITEBACK_H
#include <linux/types.h>
struct address_space;

enum writeback_sync_modes { WB_SYNC_NONE, WB_SYNC_ALL };

/* wb_reason — az ext4 indokolja a writeback-okat; a shim az értéket nem használja */
enum wb_reason {
	WB_REASON_BACKGROUND, WB_REASON_VMSCAN, WB_REASON_SYNC, WB_REASON_PERIODIC,
	WB_REASON_LAPTOP_TIMER, WB_REASON_FS_FREE_SPACE, WB_REASON_FORKER_THREAD,
	WB_REASON_FOREIGN_FLUSH, WB_REASON_INTEGRITY, WB_REASON_MAX,
};

struct writeback_control {
	long nr_to_write;
	long pages_skipped;
	loff_t range_start;
	loff_t range_end;
	enum writeback_sync_modes sync_mode;
	unsigned for_kupdate:1;
	unsigned for_background:1;
	unsigned for_reclaim:1;
	unsigned range_cyclic:1;
	unsigned tagged_writepages:1;
	unsigned for_sync:1;
};

int sync_inode_metadata(struct inode *inode, int wait);
int filemap_fdatawrite_range(struct address_space *mapping, loff_t start, loff_t end);
int filemap_flush(struct address_space *mapping);
#endif

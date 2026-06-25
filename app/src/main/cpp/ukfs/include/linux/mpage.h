/* uKernel hamis <linux/mpage.h> — multi-page I/O (a get_block_t a buffer_head.h-ból). */
#ifndef _UK_LINUX_MPAGE_H
#define _UK_LINUX_MPAGE_H
#include <linux/buffer_head.h>   /* get_block_t (függvény-típus) */
struct address_space;
struct writeback_control;
struct readahead_control;
struct folio;

void mpage_readahead(struct readahead_control *, get_block_t get_block);
int mpage_read_folio(struct folio *folio, get_block_t get_block);
int __mpage_writepages(struct address_space *mapping, struct writeback_control *wbc, get_block_t get_block, void *data);
static inline int mpage_writepages(struct address_space *mapping,
		struct writeback_control *wbc, get_block_t get_block)
{ return __mpage_writepages(mapping, wbc, get_block, NULL); }
#endif

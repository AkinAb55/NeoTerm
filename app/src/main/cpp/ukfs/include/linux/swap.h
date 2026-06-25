#ifndef _UK_LINUX_SWAP_H
#define _UK_LINUX_SWAP_H
#include <linux/types.h>
struct folio; struct page;
static inline void folio_mark_accessed(struct folio *f) { (void)f; }
static inline void mark_page_accessed(struct page *p) { (void)p; }
#endif

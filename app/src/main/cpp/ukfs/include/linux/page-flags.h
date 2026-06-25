/* uKernel hamis <linux/page-flags.h> — lap-flag stubok. */
#ifndef _UK_LINUX_PAGE_FLAGS_H
#define _UK_LINUX_PAGE_FLAGS_H
struct page; struct folio;
static inline int PageUptodate(struct page *p) { (void)p; return 1; }
static inline void SetPageUptodate(struct page *p) { (void)p; }
static inline void ClearPageUptodate(struct page *p) { (void)p; }
static inline int PageError(struct page *p) { (void)p; return 0; }
static inline int PageLocked(struct page *p) { (void)p; return 0; }
static inline int folio_test_uptodate(struct folio *f) { (void)f; return 1; }
#endif

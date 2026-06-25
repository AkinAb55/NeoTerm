#ifndef _UK_LINUX_FOLIO_BATCH_H
#define _UK_LINUX_FOLIO_BATCH_H
#include <linux/types.h>
#define FOLIO_BATCH_SIZE 31
struct folio_batch { unsigned char nr; unsigned char i; bool percpu_pvec_drained; struct folio *folios[FOLIO_BATCH_SIZE]; };
static inline void folio_batch_init(struct folio_batch *fb) { fb->nr = 0; fb->i = 0; }
static inline void folio_batch_release(struct folio_batch *fb) { fb->nr = 0; }
static inline unsigned folio_batch_count(struct folio_batch *fb) { return fb->nr; }
#endif

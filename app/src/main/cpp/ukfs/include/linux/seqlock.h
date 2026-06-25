#ifndef _UK_LINUX_SEQLOCK_H
#define _UK_LINUX_SEQLOCK_H
#include <linux/spinlock.h>
typedef struct { unsigned seq; } seqlock_t;
typedef struct { unsigned seq; } seqcount_t;
#define seqlock_init(s) do { (s)->seq = 0; } while (0)
static inline unsigned read_seqbegin(const seqlock_t *s) { return s->seq; }
static inline unsigned read_seqretry(const seqlock_t *s, unsigned start) { (void)start; (void)s; return 0; }
static inline void write_seqlock(seqlock_t *s) { (void)s; }
static inline void write_sequnlock(seqlock_t *s) { (void)s; }
#endif

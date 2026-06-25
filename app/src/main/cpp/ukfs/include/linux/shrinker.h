#ifndef _UK_LINUX_SHRINKER_H
#define _UK_LINUX_SHRINKER_H
#include <linux/types.h>
struct shrink_control { gfp_t gfp_mask; int nid; unsigned long nr_to_scan; unsigned long nr_scanned; void *memcg; };
struct shrinker { unsigned long (*count_objects)(struct shrinker *, struct shrink_control *); unsigned long (*scan_objects)(struct shrinker *, struct shrink_control *); int seeks; long batch; unsigned int flags; void *private_data; };
#define SHRINK_STOP (~0UL)
#define DEFAULT_SEEKS 2
static inline struct shrinker *shrinker_alloc(unsigned int flags, const char *fmt, ...) { (void)flags;(void)fmt; static struct shrinker s; return &s; }
static inline void shrinker_register(struct shrinker *s) { (void)s; }
static inline void shrinker_free(struct shrinker *s) { (void)s; }
#endif

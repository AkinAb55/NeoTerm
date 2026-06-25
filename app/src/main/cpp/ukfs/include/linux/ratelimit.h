/* uKernel hamis <linux/ratelimit.h>. */
#ifndef _UK_LINUX_RATELIMIT_H
#define _UK_LINUX_RATELIMIT_H
#include <linux/ratelimit_types.h>
static inline int __ratelimit_inline(struct ratelimit_state *rs) { (void)rs; return 1; }
#define ratelimit_state_init(rs, i, b) do { (rs)->interval=(i); (rs)->burst=(b); } while (0)
#endif

/* uKernel hamis <linux/ratelimit_types.h>. */
#ifndef _UK_LINUX_RATELIMIT_TYPES_H
#define _UK_LINUX_RATELIMIT_TYPES_H
#include <linux/types.h>
#ifndef _UK_RATELIMIT_STATE
#define _UK_RATELIMIT_STATE
struct ratelimit_state { int interval; int burst; int printed; int missed; unsigned long begin; unsigned long flags; };
#endif
#define DEFAULT_RATELIMIT_INTERVAL (5 * 100)
#define DEFAULT_RATELIMIT_BURST 10
#define RATELIMIT_STATE_INIT(name, intv, brst) { intv, brst, 0, 0, 0, 0 }
#define DEFINE_RATELIMIT_STATE(name, intv, brst) struct ratelimit_state name = RATELIMIT_STATE_INIT(name, intv, brst)
int __ratelimit(struct ratelimit_state *rs);
#endif

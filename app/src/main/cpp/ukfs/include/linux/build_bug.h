/* uKernel hamis <linux/build_bug.h>. */
#ifndef _UK_LINUX_BUILD_BUG_H
#define _UK_LINUX_BUILD_BUG_H
#define BUILD_BUG_ON(cond) ((void)sizeof(char[1 - 2*!!(cond)]))
#define BUILD_BUG_ON_MSG(cond, msg) BUILD_BUG_ON(cond)
#define BUILD_BUG_ON_ZERO(e) (0)
#define BUILD_BUG() do {} while (0)
#ifndef static_assert
#define static_assert(expr, ...) _Static_assert(expr, #expr)
#endif
#endif

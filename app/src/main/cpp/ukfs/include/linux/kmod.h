#ifndef _UK_LINUX_KMOD_H
#define _UK_LINUX_KMOD_H
#include <linux/types.h>
static inline int __request_module(bool wait, const char *fmt, ...) { (void)wait; (void)fmt; return 0; }
#define request_module(mod...) __request_module(true, mod)
#define try_then_request_module(x, mod...) (x)
#endif

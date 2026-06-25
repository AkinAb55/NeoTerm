/* uKernel hamis <linux/time64.h> — timespec64 userspace-stub. */
#ifndef _UK_LINUX_TIME64_H
#define _UK_LINUX_TIME64_H
#include <linux/types.h>

typedef long long time64_t;
struct timespec64 { time64_t tv_sec; long tv_nsec; };

#define NSEC_PER_SEC	1000000000L
#define NSEC_PER_MSEC	1000000L
#define MSEC_PER_SEC	1000L

static inline struct timespec64 ns_to_timespec64(s64 ns)
{ struct timespec64 t; t.tv_sec = ns / NSEC_PER_SEC; t.tv_nsec = ns % NSEC_PER_SEC; return t; }
static inline s64 timespec64_to_ns(const struct timespec64 *t)
{ return (s64)t->tv_sec * NSEC_PER_SEC + t->tv_nsec; }
#endif

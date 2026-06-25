#ifndef _UK_LINUX_KTIME_H
#define _UK_LINUX_KTIME_H
#include <linux/types.h>
typedef s64 ktime_t;
#define NSEC_PER_USEC	1000L
#define NSEC_PER_MSEC	1000000L
#define NSEC_PER_SEC	1000000000L
#define USEC_PER_MSEC	1000L
#define USEC_PER_SEC	1000000L
#define MSEC_PER_SEC	1000L
static inline ktime_t ns_to_ktime(u64 ns){ return (ktime_t)ns; }
static inline ktime_t ktime_set(s64 secs, unsigned long nsecs){ return secs*NSEC_PER_SEC + nsecs; }
static inline ktime_t ktime_get_boottime(void){ return 0; }
static inline ktime_t ktime_get_real(void){ return 0; }
static inline u64 ktime_to_us(ktime_t k){ return (u64)k/1000; }
static inline u64 ktime_to_ns(ktime_t k){ return (u64)k; }
#endif

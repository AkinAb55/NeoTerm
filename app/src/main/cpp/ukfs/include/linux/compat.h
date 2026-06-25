/* uKernel hamis <linux/compat.h> — 32/64-bit kompatibilitás stub. */
#ifndef _UK_LINUX_COMPAT_H
#define _UK_LINUX_COMPAT_H
#include <linux/types.h>
typedef u32 compat_ulong_t;
typedef s32 compat_long_t;
typedef u32 compat_uptr_t;
typedef s32 compat_int_t;
typedef u32 compat_uint_t;
static inline void *compat_ptr(compat_uptr_t uptr) { return (void *)(unsigned long)uptr; }
#define COMPAT_USE_64BIT_TIME 0
struct compat_flock { short l_type; short l_whence; compat_long_t l_start; compat_long_t l_len; int l_pid; };
#endif

#ifndef _UK_LINUX_MATH64_H
#define _UK_LINUX_MATH64_H
#include <linux/types.h>
static inline u64 div_u64(u64 dividend, u32 divisor){ return divisor ? dividend / divisor : 0; }
static inline u64 div64_u64(u64 dividend, u64 divisor){ return divisor ? dividend / divisor : 0; }
static inline s64 div64_s64(s64 dividend, s64 divisor){ return divisor ? dividend / divisor : 0; }
static inline u64 div_u64_rem(u64 dividend, u32 divisor, u32 *rem){ *rem = divisor ? dividend % divisor : 0; return divisor ? dividend / divisor : 0; }
static inline u64 mul_u64_u32_div(u64 a, u32 mul, u32 div){ return div ? (a * mul) / div : 0; }
#endif

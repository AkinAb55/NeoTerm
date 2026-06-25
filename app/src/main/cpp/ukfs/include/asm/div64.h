/* uKernel hamis <asm/div64.h> — 64-bites osztás-segédek (userspace: natív / és %). */
#ifndef _UK_ASM_DIV64_H
#define _UK_ASM_DIV64_H
#include <linux/types.h>
#define do_div(n, base) ({ u32 __rem = (u32)((u64)(n) % (u32)(base)); (n) = (u64)(n) / (u32)(base); __rem; })
static inline u64 div_u64(u64 dividend, u32 divisor) { return dividend / divisor; }
static inline u64 div_u64_rem(u64 dividend, u32 divisor, u32 *rem) { *rem = dividend % divisor; return dividend / divisor; }
static inline s64 div_s64(s64 dividend, s32 divisor) { return dividend / divisor; }
static inline s64 div_s64_rem(s64 dividend, s32 divisor, s32 *rem) { *rem = dividend % divisor; return dividend / divisor; }
static inline u64 div64_u64(u64 dividend, u64 divisor) { return dividend / divisor; }
static inline u64 div64_u64_rem(u64 dividend, u64 divisor, u64 *rem) { *rem = dividend % divisor; return dividend / divisor; }
static inline s64 div64_s64(s64 dividend, s64 divisor) { return dividend / divisor; }
static inline u64 mul_u32_u32(u32 a, u32 b) { return (u64)a * b; }
#endif

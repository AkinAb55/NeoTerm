/* uKernel hamis <linux/overflow.h> — a size_mul/size_add/struct_size a kernel.h-ban van. */
#ifndef _UK_LINUX_OVERFLOW_H
#define _UK_LINUX_OVERFLOW_H
#include <linux/types.h>
#include <linux/kernel.h>   /* size_mul/size_add/struct_size */
#ifndef check_add_overflow
#define check_add_overflow(a, b, d) __builtin_add_overflow(a, b, d)
#define check_mul_overflow(a, b, d) __builtin_mul_overflow(a, b, d)
#define check_sub_overflow(a, b, d) __builtin_sub_overflow(a, b, d)
#endif
#ifndef array_size
#define array_size(a, b)      size_mul(a, b)
#define array3_size(a, b, c)  size_mul(size_mul(a, b), c)
#endif
#ifndef flex_array_size
#define flex_array_size(p, member, n) size_mul(sizeof(*(p)->member), (n))
#endif
#ifndef size_sub
static inline size_t size_sub(size_t a, size_t b) { return a - b; }
#endif

/* DEFINE_RAW_FLEX(type, name, member, count): stack-változó rugalmas tömbbel a végén.
 * Az ext4 mballoc a seq-statisztikához használ ilyet (sg). */
#ifndef DEFINE_RAW_FLEX
#define DEFINE_RAW_FLEX(type, name, member, count) \
	unsigned char __raw_##name[offsetof(type, member) + sizeof(((type *)0)->member[0]) * (count)]; \
	type *name = (void *)__raw_##name
#endif
#ifndef DEFINE_FLEX
#define DEFINE_FLEX(type, name, member, COUNTER, count) \
	DEFINE_RAW_FLEX(type, name, member, count)
#endif
#endif

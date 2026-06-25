/* uKernel hamis <linux/log2.h> — ilog2/roundup_pow_of_two/order_base_2. */
#ifndef _UK_LINUX_LOG2_H
#define _UK_LINUX_LOG2_H

#include <linux/types.h>
#include <linux/bitops.h>

static inline __attribute__((const))
int __ilog2_u32(u32 n)
{
	return 31 - __builtin_clz(n | 1);
}

static inline __attribute__((const))
int __ilog2_u64(u64 n)
{
	return 63 - __builtin_clzll(n | 1);
}

#define ilog2(n)						\
(								\
	__builtin_constant_p(n) ?				\
	((n) < 2 ? 0 : 63 - __builtin_clzll((unsigned long long)(n))) : \
	(sizeof(n) <= 4) ? __ilog2_u32(n) : __ilog2_u64(n)	\
)

static inline __attribute__((const))
unsigned long __roundup_pow_of_two(unsigned long n)
{
	if (n < 2)
		return 1;
	return 1UL << (ilog2(n - 1) + 1);
}

static inline __attribute__((const))
unsigned long __rounddown_pow_of_two(unsigned long n)
{
	return 1UL << ilog2(n);
}

#define roundup_pow_of_two(n)					\
(								\
	__builtin_constant_p(n) ? (				\
		((n) == 1) ? 1 :				\
		(1UL << (ilog2((n) - 1) + 1))			\
	) : __roundup_pow_of_two(n)				\
)

#define rounddown_pow_of_two(n)					\
(								\
	__builtin_constant_p(n) ? (				\
		(1UL << ilog2(n))				\
	) : __rounddown_pow_of_two(n)				\
)

#define order_base_2(n)						\
(								\
	((n) < 2) ? 0 : (ilog2((n) - 1) + 1)			\
)

#define is_power_of_2(n) ((n) != 0 && (((n) & ((n) - 1)) == 0))

#define bits_per(n)						\
(								\
	((n) < 2) ? 1 : (ilog2((n) - 1) + 1)			\
)

#endif

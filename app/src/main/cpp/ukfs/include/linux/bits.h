#ifndef _UK_LINUX_BITS_H
#define _UK_LINUX_BITS_H
#include <linux/types.h>
#include <linux/bitops.h>
#ifndef __BITS_PER_LONG
#define __BITS_PER_LONG 64
#endif
#ifndef BIT
#define BIT(n) (1UL << (n))
#endif
#define BIT_ULL(n) (1ULL << (n))
#define BIT_MASK(nr) (1UL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr) ((nr) / BITS_PER_LONG)
#ifndef GENMASK
#define GENMASK(h, l) (((~0UL) << (l)) & (~0UL >> (BITS_PER_LONG - 1 - (h))))
#endif
#define GENMASK_ULL(h, l) (((~0ULL) << (l)) & (~0ULL >> (63 - (h))))
#endif

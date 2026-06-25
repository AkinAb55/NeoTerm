/* uKernel hamis <linux/hash.h> — egész-hash userspace-stub. */
#ifndef _UK_LINUX_HASH_H
#define _UK_LINUX_HASH_H
#include <linux/types.h>

#define GOLDEN_RATIO_32 0x61C88647
#define GOLDEN_RATIO_64 0x61C8864680B583EBull

static inline u32 hash_32(u32 val, unsigned int bits)
{ return (val * GOLDEN_RATIO_32) >> (32 - bits); }
static inline u32 hash_64(u64 val, unsigned int bits)
{ return (u32)((val * GOLDEN_RATIO_64) >> (64 - bits)); }
static inline u32 hash_long(unsigned long val, unsigned int bits)
{ return hash_64((u64)val, bits); }
#endif

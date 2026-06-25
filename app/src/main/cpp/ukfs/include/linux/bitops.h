/* uKernel shim — bitops: a tényleges bit-műveletek az atomic.h-ban vannak. */
#ifndef _UKNL_BITOPS_H
#define _UKNL_BITOPS_H
#include <linux/atomic.h>
#include <linux/types.h>
#ifndef BIT
#define BIT(n) (1UL << (n))
#endif
#ifndef BITS_PER_LONG
#define BITS_PER_LONG 64
#endif
#define BITS_PER_BYTE 8
#define BITS_TO_LONGS(n) (((n) + 63) / 64)
#define DECLARE_BITMAP(name, bits) unsigned long name[BITS_TO_LONGS(bits)]
#define GENMASK(h, l) (((~0UL) << (l)) & (~0UL >> (BITS_PER_LONG - 1 - (h))))
static inline unsigned int hweight8(uint8_t w){ unsigned c=0; while(w){c+=w&1;w>>=1;} return c; }
static inline unsigned int hweight16(uint16_t w){ unsigned c=0; while(w){c+=w&1;w>>=1;} return c; }
static inline unsigned int hweight32(uint32_t w){ unsigned c=0; while(w){c+=w&1;w>>=1;} return c; }
static inline int fls(int x){ int r=0; while(x){r++;x>>=1;} return r; }

static inline void bitmap_zero(unsigned long *dst, unsigned nbits)
{
	unsigned i, n = BITS_TO_LONGS(nbits);
	for (i = 0; i < n; i++) dst[i] = 0UL;
}
static inline void bitmap_fill(unsigned long *dst, unsigned nbits)
{
	unsigned i, n = BITS_TO_LONGS(nbits);
	for (i = 0; i < n; i++) dst[i] = ~0UL;
}
static inline void bitmap_copy(unsigned long *dst, const unsigned long *src,
			       unsigned nbits)
{
	unsigned i, n = BITS_TO_LONGS(nbits);
	for (i = 0; i < n; i++) dst[i] = src[i];
}
static inline int bitmap_empty(const unsigned long *src, unsigned nbits)
{
	unsigned i, n = BITS_TO_LONGS(nbits);
	for (i = 0; i < n; i++) if (src[i]) return 0;
	return 1;
}
#define ffz(x) __builtin_ctzl(~(unsigned long)(x))
#define hweight_long(x) __builtin_popcountl((unsigned long)(x))
#define BITMAP_LAST_WORD_MASK(nbits) (~0UL >> (-(nbits) & (BITS_PER_LONG-1)))
static inline void clear_bit_le(int nr, void *addr){ ((unsigned char*)addr)[nr>>3] &= ~(1<<(nr&7)); }
static inline void set_bit_le(int nr, void *addr){ ((unsigned char*)addr)[nr>>3] |= (1<<(nr&7)); }
static inline int test_bit_le(int nr, const void *addr){ return (((const unsigned char*)addr)[nr>>3] >> (nr&7)) & 1; }
#define BIT_WORD(nr) ((nr)/64)
#define BITS_TO_U64(nr) (((nr)+63)/64)
#define BITMAP_FIRST_WORD_MASK(start) (~0UL << ((start)&63))
static inline unsigned int blksize_bits(unsigned int size){ unsigned int b=9; while(size>(1u<<b))b++; return b; }
#endif

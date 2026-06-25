#ifndef _UK_LINUX_BITREV_H
#define _UK_LINUX_BITREV_H
#include <linux/types.h>
static inline u8 bitrev8(u8 x){ x=(x>>4)|(x<<4); x=((x&0xCC)>>2)|((x&0x33)<<2); x=((x&0xAA)>>1)|((x&0x55)<<1); return x; }
static inline u16 bitrev16(u16 x){ return (bitrev8(x&0xff)<<8)|bitrev8(x>>8); }
static inline u32 bitrev32(u32 x){ return ((u32)bitrev16(x&0xffff)<<16)|bitrev16(x>>16); }
#endif

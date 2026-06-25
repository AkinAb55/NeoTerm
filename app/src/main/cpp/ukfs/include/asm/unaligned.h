#ifndef _UK_ASM_UNALIGNED_H
#define _UK_ASM_UNALIGNED_H
#include <linux/types.h>
static inline u16 get_unaligned_le16(const void *p){ const u8 *b=p; return b[0]|(b[1]<<8); }
static inline u32 get_unaligned_le32(const void *p){ const u8 *b=p; return b[0]|(b[1]<<8)|(b[2]<<16)|((u32)b[3]<<24); }
static inline void put_unaligned_le16(u16 v, void *p){ u8 *b=p; b[0]=v; b[1]=v>>8; }
static inline void put_unaligned_le32(u32 v, void *p){ u8 *b=p; b[0]=v; b[1]=v>>8; b[2]=v>>16; b[3]=v>>24; }
static inline u16 get_unaligned_be16(const void *p){ const u8 *b=p; return ((u16)b[0]<<8)|b[1]; }
static inline u32 get_unaligned_be32(const void *p){ const u8 *b=p; return ((u32)b[0]<<24)|((u32)b[1]<<16)|((u32)b[2]<<8)|b[3]; }
static inline void put_unaligned_be16(u16 v, void *p){ u8 *b=p; b[0]=v>>8; b[1]=v; }
static inline void put_unaligned_be32(u32 v, void *p){ u8 *b=p; b[0]=v>>24; b[1]=v>>16; b[2]=v>>8; b[3]=v; }
static inline u64 get_unaligned_le64(const void *p){ const u8 *b=p; return get_unaligned_le32(b) | ((u64)get_unaligned_le32(b+4) << 32); }
static inline void put_unaligned_le64(u64 v, void *p){ u8 *b=p; put_unaligned_le32((u32)v, b); put_unaligned_le32((u32)(v>>32), b+4); }
static inline u64 get_unaligned_be64(const void *p){ const u8 *b=p; return ((u64)get_unaligned_be32(b) << 32) | get_unaligned_be32(b+4); }
static inline void put_unaligned_be64(u64 v, void *p){ u8 *b=p; put_unaligned_be32((u32)(v>>32), b); put_unaligned_be32((u32)v, b+4); }

/* generikus get/put_unaligned — tipus-fuggo (memcpy semantika) */
#define get_unaligned(ptr) \
	__builtin_choose_expr(sizeof(*(ptr)) == 1, *(ptr), \
	({ typeof(*(ptr)) __v; __builtin_memcpy(&__v, (ptr), sizeof(__v)); __v; }))
#define put_unaligned(val, ptr) \
	do { typeof(*(ptr)) __v = (val); __builtin_memcpy((ptr), &__v, sizeof(__v)); } while (0)
#endif

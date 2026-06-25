#ifndef _UK_ASM_IO_H
#define _UK_ASM_IO_H
#include <linux/types.h>
static inline u8  readb(const volatile void *a){ return *(const volatile u8 *)a; }
static inline u16 readw(const volatile void *a){ return *(const volatile u16 *)a; }
static inline u32 readl(const volatile void *a){ return *(const volatile u32 *)a; }
static inline void writeb(u8 v, volatile void *a){ *(volatile u8 *)a=v; }
static inline void writew(u16 v, volatile void *a){ *(volatile u16 *)a=v; }
static inline void writel(u32 v, volatile void *a){ *(volatile u32 *)a=v; }
#endif

/* uKernel hamis <linux/crc32.h> — CRC-32 (IEEE, LE) tabla nelkuli megvalositas. */
#ifndef _UK_LINUX_CRC32_H
#define _UK_LINUX_CRC32_H
#include <linux/types.h>
u32 crc32_le(u32 crc, const u8 *p, size_t len);
u32 crc32_be(u32 crc, const u8 *p, size_t len);
u32 __crc32c_le(u32 crc, const u8 *p, size_t len);
#define crc32(seed, data, length) crc32_le(seed, (const u8 *)(data), length)
#define crc32c(seed, data, length) __crc32c_le(seed, (const u8 *)(data), length)
static inline u32 crc32_le_combine(u32 crc1, u32 crc2, size_t len2)
{ (void)crc2; (void)len2; return crc1; }
#endif

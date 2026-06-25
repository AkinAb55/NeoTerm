/* uKernel hamis <linux/hex.h> — hex_to_bin / bin2hex / hex2bin. */
#ifndef _UK_LINUX_HEX_H
#define _UK_LINUX_HEX_H

#include <linux/types.h>

extern const char hex_asc[];
extern const char hex_asc_upper[];

#define hex_asc_lo(x)	hex_asc[((x) & 0x0f)]
#define hex_asc_hi(x)	hex_asc[((x) & 0xf0) >> 4]
#define hex_asc_upper_lo(x)	hex_asc_upper[((x) & 0x0f)]
#define hex_asc_upper_hi(x)	hex_asc_upper[((x) & 0xf0) >> 4]

int hex_to_bin(unsigned char ch);
int hex2bin(u8 *dst, const char *src, size_t count);
char *bin2hex(char *dst, const void *src, size_t count);

static inline char *hex_byte_pack(char *buf, u8 byte)
{
	*buf++ = hex_asc_hi(byte);
	*buf++ = hex_asc_lo(byte);
	return buf;
}

static inline char *hex_byte_pack_upper(char *buf, u8 byte)
{
	*buf++ = hex_asc_upper_hi(byte);
	*buf++ = hex_asc_upper_lo(byte);
	return buf;
}

#endif

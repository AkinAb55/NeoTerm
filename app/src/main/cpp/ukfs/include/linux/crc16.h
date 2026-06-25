#ifndef _UK_LINUX_CRC16_H
#define _UK_LINUX_CRC16_H
#include <linux/types.h>
u16 crc16(u16 crc, const u8 *buffer, size_t len);
#endif

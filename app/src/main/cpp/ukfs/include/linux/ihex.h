/* uKernel hamis <linux/ihex.h> — Intel HEX firmware rekordok stubja. */
#ifndef _UK_LINUX_IHEX_H
#define _UK_LINUX_IHEX_H

#include <linux/types.h>
#include <linux/firmware.h>
#include <asm/byteorder.h>

struct ihex_binrec {
	__be32 addr;
	__be16 len;
	uint8_t data[];
} __attribute__((packed));

static inline uint16_t ihex_binrec_size(const struct ihex_binrec *p)
{
	return be16_to_cpu(p->len) + sizeof(*p);
}

static inline const struct ihex_binrec *
ihex_next_binrec(const struct ihex_binrec *rec)
{
	int next = ((be16_to_cpu(rec->len) + 5) & ~3) - 2;
	rec = (const void *)&rec->data[next];
	return be16_to_cpu(rec->len) ? rec : NULL;
}

static inline int ihex_validate_fw(const struct firmware *fw)
{
	(void)fw;
	return 0;
}

static inline int request_ihex_firmware(const struct firmware **fw,
					const char *fw_name,
					struct device *dev)
{
	return request_firmware(fw, fw_name, dev);
}

#endif

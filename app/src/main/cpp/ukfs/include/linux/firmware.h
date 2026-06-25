/* uKernel hamis <linux/firmware.h> — firmware-betöltés stub (compile-only). */
#ifndef _UK_LINUX_FIRMWARE_H
#define _UK_LINUX_FIRMWARE_H

#include <linux/types.h>

struct device;

struct firmware {
	size_t size;
	const u8 *data;
};

int  request_firmware(const struct firmware **fw, const char *name,
		      struct device *device);
int  firmware_request_nowarn(const struct firmware **fw, const char *name,
			     struct device *device);
int  request_firmware_direct(const struct firmware **fw, const char *name,
			     struct device *device);
void release_firmware(const struct firmware *fw);

#endif

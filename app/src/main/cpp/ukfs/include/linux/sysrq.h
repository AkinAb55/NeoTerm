/* uKernel hamis <linux/sysrq.h> — a usb-serial CONSOLE/SYSRQ útja kikapcsolt,
 * így csak a handle_sysrq() prototípus kell a fordításhoz. */
#ifndef _UK_LINUX_SYSRQ_H
#define _UK_LINUX_SYSRQ_H

void handle_sysrq(int key);

#endif

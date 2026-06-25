/* uKernel hamis <linux/usb/input.h> — usb_to_input_id() segedfuggveny. */
#ifndef _UK_LINUX_USB_INPUT_H
#define _UK_LINUX_USB_INPUT_H

#include <linux/usb.h>
#include <linux/input.h>

static inline void
usb_to_input_id(const struct usb_device *dev, struct input_id *id)
{
	id->bustype = BUS_USB;
	id->vendor = le16_to_cpu(dev->descriptor.idVendor);
	id->product = le16_to_cpu(dev->descriptor.idProduct);
	id->version = le16_to_cpu(dev->descriptor.bcdDevice);
}

#endif

/* uKernel hamis <linux/usb/ezusb.h> — Cypress EZ-USB firmware stub. */
#ifndef _UK_LINUX_USB_EZUSB_H
#define _UK_LINUX_USB_EZUSB_H

struct usb_device;

int ezusb_fx1_set_reset(struct usb_device *dev, unsigned char reset_bit);
int ezusb_fx1_ihex_firmware_download(struct usb_device *dev,
				     const char *firmware_path);

#endif

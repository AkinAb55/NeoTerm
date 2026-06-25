/* uKernel hamis <linux/usb/hcd.h> — a usb-storage csak bus_to_hcd()-t es a
 * localmem_pool mezot, valamint hcd_uses_dma()-t hasznalja. A userspace
 * libusb-modellben nincs valodi HCD; ezek no-op/NULL ertekek. */
#ifndef _UK_LINUX_USB_HCD_H
#define _UK_LINUX_USB_HCD_H

#include <linux/usb.h>

struct hc_driver {
	const char	*description;
};

struct usb_hcd {
	void			*localmem_pool;	/* mindig NULL ebben a modellben */
	const struct hc_driver	*driver;
	unsigned		can_do_streams:1;
};

static inline struct usb_hcd *bus_to_hcd(struct usb_bus *bus)
{ (void)bus; return NULL; }

static inline struct usb_bus *hcd_to_bus(struct usb_hcd *hcd)
{ (void)hcd; return NULL; }

static inline bool hcd_uses_dma(struct usb_hcd *hcd)
{ (void)hcd; return false; }

#endif

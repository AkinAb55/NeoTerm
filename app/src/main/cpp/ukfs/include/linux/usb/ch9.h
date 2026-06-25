/* uKernel hamis <linux/usb/ch9.h> — USB 2.0 spec descriptorok és konstansok. */
#ifndef _UK_LINUX_USB_CH9_H
#define _UK_LINUX_USB_CH9_H

#include <linux/types.h>

/* bmRequestType irány/típus/cél */
#define USB_DIR_OUT        0x00
#define USB_DIR_IN         0x80
#define USB_TYPE_STANDARD  0x00
#define USB_TYPE_CLASS     0x20
#define USB_TYPE_VENDOR    0x40
#define USB_RECIP_DEVICE   0x00
#define USB_RECIP_INTERFACE 0x01
#define USB_RECIP_ENDPOINT 0x02
#define USB_RECIP_OTHER    0x03
#define USB_RECIP_PORT     0x04
#define USB_RECIP_RPIPE    0x05

/* eszköz/interfész osztályok (bDeviceClass / bInterfaceClass) */
#define USB_CLASS_PER_INTERFACE		0x00
#define USB_CLASS_AUDIO			0x01
#define USB_CLASS_COMM			0x02
#define USB_CLASS_HID			0x03
#define USB_CLASS_PHYSICAL		0x05
#define USB_CLASS_STILL_IMAGE		0x06
#define USB_CLASS_PRINTER		0x07
#define USB_CLASS_MASS_STORAGE		0x08
#define USB_CLASS_HUB			0x09
#define USB_CLASS_CDC_DATA		0x0a
#define USB_CLASS_CSCID			0x0b
#define USB_CLASS_CONTENT_SEC		0x0d
#define USB_CLASS_VIDEO			0x0e
#define USB_CLASS_WIRELESS_CONTROLLER	0xe0
#define USB_CLASS_MISC			0xef
#define USB_CLASS_APP_SPEC		0xfe
#define USB_CLASS_VENDOR_SPEC		0xff

#define USB_SUBCLASS_VENDOR_SPEC	0xff
#define USB_SUBCLASS_IRDA		0x02

/* standard kérések */
#define USB_REQ_GET_STATUS        0x00
#define USB_REQ_CLEAR_FEATURE     0x01
#define USB_REQ_SET_FEATURE       0x03
#define USB_REQ_SET_ADDRESS       0x05
#define USB_REQ_GET_DESCRIPTOR    0x06
#define USB_REQ_SET_DESCRIPTOR    0x07
#define USB_REQ_GET_CONFIGURATION 0x08
#define USB_REQ_SET_CONFIGURATION 0x09
#define USB_REQ_GET_INTERFACE     0x0A
#define USB_REQ_SET_INTERFACE     0x0B

/* descriptor típusok */
#define USB_DT_DEVICE      0x01
#define USB_DT_CONFIG      0x02
#define USB_DT_STRING      0x03
#define USB_DT_INTERFACE   0x04
#define USB_DT_ENDPOINT    0x05
#define USB_DT_CS_DEVICE	(USB_TYPE_CLASS | USB_DT_DEVICE)
#define USB_DT_CS_CONFIG	(USB_TYPE_CLASS | USB_DT_CONFIG)
#define USB_DT_CS_STRING	(USB_TYPE_CLASS | USB_DT_STRING)
#define USB_DT_CS_INTERFACE	(USB_TYPE_CLASS | USB_DT_INTERFACE)
#define USB_DT_CS_ENDPOINT	(USB_TYPE_CLASS | USB_DT_ENDPOINT)
#define USB_CONFIG_ATT_ONE	(1 << 7)
#define USB_CONFIG_ATT_SELFPOWER (1 << 6)
#define USB_CONFIG_ATT_WAKEUP	(1 << 5)
#define USB_CONFIG_ATT_BATTERY	(1 << 4)

/* endpoint attribútumok */
#define USB_ENDPOINT_NUMBER_MASK   0x0f
#define USB_ENDPOINT_DIR_MASK      0x80
#ifndef USB_ENDPOINT_HALT
#define USB_ENDPOINT_HALT          0	/* feature-selector: ep stall torlese */
#endif
#define USB_ENDPOINT_XFERTYPE_MASK 0x03
#define USB_ENDPOINT_XFER_CONTROL  0
#define USB_ENDPOINT_XFER_ISOC     1
#define USB_ENDPOINT_XFER_BULK     2
#define USB_ENDPOINT_XFER_INT      3

struct usb_ctrlrequest {
	__u8  bRequestType;
	__u8  bRequest;
	__le16 wValue;
	__le16 wIndex;
	__le16 wLength;
} __attribute__((packed));

struct usb_device_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;
	__le16 bcdUSB;
	__u8  bDeviceClass;
	__u8  bDeviceSubClass;
	__u8  bDeviceProtocol;
	__u8  bMaxPacketSize0;
	__le16 idVendor;
	__le16 idProduct;
	__le16 bcdDevice;
	__u8  iManufacturer;
	__u8  iProduct;
	__u8  iSerialNumber;
	__u8  bNumConfigurations;
} __attribute__((packed));

struct usb_config_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;
	__le16 wTotalLength;
	__u8  bNumInterfaces;
	__u8  bConfigurationValue;
	__u8  iConfiguration;
	__u8  bmAttributes;
	__u8  bMaxPower;
} __attribute__((packed));

struct usb_interface_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;
	__u8  bInterfaceNumber;
	__u8  bAlternateSetting;
	__u8  bNumEndpoints;
	__u8  bInterfaceClass;
	__u8  bInterfaceSubClass;
	__u8  bInterfaceProtocol;
	__u8  iInterface;
} __attribute__((packed));

struct usb_endpoint_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;
	__u8  bEndpointAddress;
	__u8  bmAttributes;
	__le16 wMaxPacketSize;
	__u8  bInterval;
} __attribute__((packed));

#ifndef USB_DT_SS_ENDPOINT_COMP
#define USB_DT_SS_ENDPOINT_COMP	0x30
#endif
#ifndef USB_DT_PIPE_USAGE
#define USB_DT_PIPE_USAGE	0x24	/* UAS pipe-usage leiro */
#endif

/* SuperSpeed endpoint companion leiro — a UAS stream-detektalashoz. */
struct usb_ss_ep_comp_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;
	__u8  bMaxBurst;
	__u8  bmAttributes;
	__le16 wBytesPerInterval;
} __attribute__((packed));

/* bmAttributes & USB_SS_MULT/streams */
static inline int usb_ss_max_streams(const struct usb_ss_ep_comp_descriptor *comp)
{
	int max_streams;
	if (!comp)
		return 0;
	max_streams = comp->bmAttributes & 0x1f;
	if (!max_streams)
		return 0;
	return 1 << max_streams;
}

/* endpoint segéd-inline-ok */
static inline int usb_endpoint_num(const struct usb_endpoint_descriptor *e)
{ return e->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK; }
static inline int usb_endpoint_dir_in(const struct usb_endpoint_descriptor *e)
{ return (e->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN; }
static inline int usb_endpoint_dir_out(const struct usb_endpoint_descriptor *e)
{ return (e->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_OUT; }
static inline int usb_endpoint_type(const struct usb_endpoint_descriptor *e)
{ return e->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK; }
static inline int usb_endpoint_xfer_bulk(const struct usb_endpoint_descriptor *e)
{ return usb_endpoint_type(e) == USB_ENDPOINT_XFER_BULK; }
static inline int usb_endpoint_xfer_int(const struct usb_endpoint_descriptor *e)
{ return usb_endpoint_type(e) == USB_ENDPOINT_XFER_INT; }
static inline int usb_endpoint_xfer_isoc(const struct usb_endpoint_descriptor *e)
{ return usb_endpoint_type(e) == USB_ENDPOINT_XFER_ISOC; }
static inline int usb_endpoint_is_bulk_in(const struct usb_endpoint_descriptor *e)
{ return usb_endpoint_xfer_bulk(e) && usb_endpoint_dir_in(e); }
static inline int usb_endpoint_is_bulk_out(const struct usb_endpoint_descriptor *e)
{ return usb_endpoint_xfer_bulk(e) && usb_endpoint_dir_out(e); }
static inline int usb_endpoint_is_int_in(const struct usb_endpoint_descriptor *e)
{ return usb_endpoint_xfer_int(e) && usb_endpoint_dir_in(e); }

#endif

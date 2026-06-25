/* uKernel hamis <linux/usb/irda.h> — USB-IrDA class descriptor (ir-usb.c). */
#ifndef _UK_LINUX_USB_IRDA_H
#define _UK_LINUX_USB_IRDA_H

#include <linux/types.h>

#define USB_DT_CS_IRDA	0x21

/* class-specific kérés: IrDA descriptor lekérdezése */
#define USB_REQ_CS_IRDA_GET_CLASS_DESC	0x06

struct usb_irda_cs_descriptor {
	__u8	bLength;
	__u8	bDescriptorType;
	__le16	bcdSpecRevision;
	__u8	bmDataSize;
	__u8	bmWindowSize;
	__u8	bmMinTurnaroundTime;
	__le16	wBaudRate;
	__u8	bmAdditionalBOFs;
	__u8	bIrdaRateSniff;
	__u8	bMaxUnicastList;
} __attribute__((packed));

/* baud-rate bitek (wBaudRate) */
#define USB_IRDA_BR_2400	0x0001
#define USB_IRDA_BR_9600	0x0002
#define USB_IRDA_BR_19200	0x0004
#define USB_IRDA_BR_38400	0x0008
#define USB_IRDA_BR_57600	0x0010
#define USB_IRDA_BR_115200	0x0020
#define USB_IRDA_BR_576000	0x0040
#define USB_IRDA_BR_1152000	0x0080
#define USB_IRDA_BR_4000000	0x0100

/* line-speed kódolás */
#define USB_IRDA_LS_NO_CHANGE	0xff
#define USB_IRDA_LS_2400	0x00
#define USB_IRDA_LS_9600	0x01
#define USB_IRDA_LS_19200	0x02
#define USB_IRDA_LS_38400	0x03
#define USB_IRDA_LS_57600	0x04
#define USB_IRDA_LS_115200	0x05
#define USB_IRDA_LS_576000	0x06
#define USB_IRDA_LS_1152000	0x07
#define USB_IRDA_LS_4000000	0x08

/* additional BOFs (bmAdditionalBOFs) */
#define USB_IRDA_AB_48		0x00
#define USB_IRDA_AB_24		0x01
#define USB_IRDA_AB_12		0x02
#define USB_IRDA_AB_6		0x03
#define USB_IRDA_AB_3		0x04
#define USB_IRDA_AB_2		0x05
#define USB_IRDA_AB_1		0x06
#define USB_IRDA_AB_0		0x07

#endif

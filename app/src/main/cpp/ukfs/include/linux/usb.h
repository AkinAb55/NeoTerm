/* uKernel hamis <linux/usb.h> — a USB core driver-felülete.
 *
 * Cél: egy valódi USB driver probe-ja és I/O-ja lefusson. Az URB/pipe modellt
 * a HCD bridge-re (ukernel/hcd.h) képezzük. */
#ifndef _UK_LINUX_USB_H
#define _UK_LINUX_USB_H

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/usb/ch9.h>
#include <linux/completion.h>
#include <linux/pm.h>
#include <linux/workqueue.h>
#include <linux/scatterlist.h>
#include <linux/interrupt.h>
#include <linux/kref.h>
#include <linux/iopoll.h>

struct module;   /* forward — a usb_register_driver() owner-paraméteréhez */

/* ===== eszköz-illesztés ===== */
typedef unsigned long kernel_ulong_t;

#define USB_DEVICE_ID_MATCH_VENDOR		0x0001
#define USB_DEVICE_ID_MATCH_PRODUCT		0x0002
#define USB_DEVICE_ID_MATCH_DEV_LO		0x0004
#define USB_DEVICE_ID_MATCH_DEV_HI		0x0008
#define USB_DEVICE_ID_MATCH_DEV_CLASS		0x0010
#define USB_DEVICE_ID_MATCH_DEV_SUBCLASS	0x0020
#define USB_DEVICE_ID_MATCH_DEV_PROTOCOL	0x0040
#define USB_DEVICE_ID_MATCH_INT_CLASS		0x0080
#define USB_DEVICE_ID_MATCH_INT_SUBCLASS	0x0100
#define USB_DEVICE_ID_MATCH_INT_PROTOCOL	0x0200
#define USB_DEVICE_ID_MATCH_INT_NUMBER		0x0400

#define USB_DEVICE_ID_MATCH_DEVICE \
	(USB_DEVICE_ID_MATCH_VENDOR | USB_DEVICE_ID_MATCH_PRODUCT)
#define USB_DEVICE_ID_MATCH_DEV_RANGE \
	(USB_DEVICE_ID_MATCH_DEV_LO | USB_DEVICE_ID_MATCH_DEV_HI)
#define USB_DEVICE_ID_MATCH_DEVICE_AND_VERSION \
	(USB_DEVICE_ID_MATCH_DEVICE | USB_DEVICE_ID_MATCH_DEV_RANGE)
#define USB_DEVICE_ID_MATCH_DEV_INFO \
	(USB_DEVICE_ID_MATCH_DEV_CLASS | USB_DEVICE_ID_MATCH_DEV_SUBCLASS | \
	 USB_DEVICE_ID_MATCH_DEV_PROTOCOL)
#define USB_DEVICE_ID_MATCH_INT_INFO \
	(USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_INT_SUBCLASS | \
	 USB_DEVICE_ID_MATCH_INT_PROTOCOL)

struct usb_device_id {
	__u16 match_flags;
	__u16 idVendor;
	__u16 idProduct;
	__u16 bcdDevice_lo, bcdDevice_hi;
	__u8  bDeviceClass, bDeviceSubClass, bDeviceProtocol;
	__u8  bInterfaceClass, bInterfaceSubClass, bInterfaceProtocol, bInterfaceNumber;
	kernel_ulong_t driver_info;
};

#define USB_DEVICE(vend, prod) \
	.match_flags = USB_DEVICE_ID_MATCH_DEVICE, \
	.idVendor = (vend), .idProduct = (prod)

#define USB_DEVICE_VER(vend, prod, lo, hi) \
	.match_flags = USB_DEVICE_ID_MATCH_DEVICE_AND_VERSION, \
	.idVendor = (vend), .idProduct = (prod), \
	.bcdDevice_lo = (lo), .bcdDevice_hi = (hi)

#define USB_DEVICE_INTERFACE_CLASS(vend, prod, cl) \
	.match_flags = USB_DEVICE_ID_MATCH_DEVICE | USB_DEVICE_ID_MATCH_INT_CLASS, \
	.idVendor = (vend), .idProduct = (prod), \
	.bInterfaceClass = (cl)

#define USB_DEVICE_INTERFACE_PROTOCOL(vend, prod, pr) \
	.match_flags = USB_DEVICE_ID_MATCH_DEVICE | USB_DEVICE_ID_MATCH_INT_PROTOCOL, \
	.idVendor = (vend), .idProduct = (prod), \
	.bInterfaceProtocol = (pr)

#define USB_DEVICE_INTERFACE_NUMBER(vend, prod, num) \
	.match_flags = USB_DEVICE_ID_MATCH_DEVICE | USB_DEVICE_ID_MATCH_INT_NUMBER, \
	.idVendor = (vend), .idProduct = (prod), \
	.bInterfaceNumber = (num)

#define USB_DEVICE_AND_INTERFACE_INFO(vend, prod, cl, sc, pr) \
	.match_flags = USB_DEVICE_ID_MATCH_DEVICE | USB_DEVICE_ID_MATCH_INT_INFO, \
	.idVendor = (vend), .idProduct = (prod), \
	.bInterfaceClass = (cl), .bInterfaceSubClass = (sc), .bInterfaceProtocol = (pr)

#define USB_VENDOR_AND_INTERFACE_INFO(vend, cl, sc, pr) \
	.match_flags = USB_DEVICE_ID_MATCH_VENDOR | USB_DEVICE_ID_MATCH_INT_INFO, \
	.idVendor = (vend), \
	.bInterfaceClass = (cl), .bInterfaceSubClass = (sc), .bInterfaceProtocol = (pr)

#define USB_DEVICE_INFO(cl, sc, pr) \
	.match_flags = USB_DEVICE_ID_MATCH_DEV_INFO, \
	.bDeviceClass = (cl), .bDeviceSubClass = (sc), .bDeviceProtocol = (pr)

#define USB_INTERFACE_INFO(cl, sc, pr) \
	.match_flags = USB_DEVICE_ID_MATCH_INT_INFO, \
	.bInterfaceClass = (cl), .bInterfaceSubClass = (sc), .bInterfaceProtocol = (pr)

/* USB control kérés timeout-ok (ezredmásodperc) */
#define USB_CTRL_GET_TIMEOUT	5000
#define USB_CTRL_SET_TIMEOUT	5000

/* ===== topológia ===== */
struct usb_host_endpoint {
	struct usb_endpoint_descriptor desc;
	struct usb_ss_ep_comp_descriptor ss_ep_comp;	/* UAS stream-detekcio */
	unsigned char *extra;
	int extralen;
};

struct usb_host_interface {
	struct usb_interface_descriptor desc;
	struct usb_host_endpoint *endpoint;   /* bNumEndpoints elem */
	unsigned char *extra;
	int extralen;
};

struct usb_interface {
	struct usb_host_interface *altsetting;     /* tömb */
	struct usb_host_interface *cur_altsetting;
	unsigned num_altsetting;
	int minor;
	unsigned needs_binding:1;
	unsigned needs_remote_wakeup:1;
	struct device dev;
	struct usb_device *usb_dev;                 /* uKernel: visszamutató */
	struct usb_interface_assoc_descriptor *intf_assoc;
	void *intfdata;
};

struct usb_interface_assoc_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;
	__u8  bFirstInterface;
	__u8  bInterfaceCount;
	__u8  bFunctionClass;
	__u8  bFunctionSubClass;
	__u8  bFunctionProtocol;
	__u8  iFunction;
} __attribute__ ((packed));

struct usb_interface_cache {
	unsigned num_altsetting;
	struct kref ref;
	struct usb_host_interface *altsetting;
};

struct usb_device_driver {
	const char *name;
	int  (*probe)(struct usb_device *udev);
	void (*disconnect)(struct usb_device *udev);
	int  (*suspend)(struct usb_device *udev, pm_message_t message);
	int  (*resume)(struct usb_device *udev, pm_message_t message);
	int  (*choose_configuration)(struct usb_device *udev);
	const struct usb_device_id *id_table;
	unsigned int supports_autosuspend:1;
	unsigned int generic_subclass:1;
};
int  usb_register_device_driver(struct usb_device_driver *drv, struct module *owner);
void usb_deregister_device_driver(struct usb_device_driver *drv);

struct usb_host_config {
	struct usb_config_descriptor desc;
	struct usb_interface *interface[16];
	struct usb_interface_cache *intf_cache[16];
	unsigned char *extra;
	int extralen;
};

enum usb_device_speed { USB_SPEED_UNKNOWN=0, USB_SPEED_LOW, USB_SPEED_FULL, USB_SPEED_HIGH, USB_SPEED_WIRELESS, USB_SPEED_SUPER, USB_SPEED_SUPER_PLUS };
enum usb_device_state { USB_STATE_NOTATTACHED=0, USB_STATE_ATTACHED, USB_STATE_POWERED,
	USB_STATE_DEFAULT, USB_STATE_ADDRESS, USB_STATE_CONFIGURED, USB_STATE_SUSPENDED };

struct usb_bus {
	const char *bus_name;
	int busnum;
	struct device *sysdev;		/* usb-storage: DMA-szulo eszkoz */
	unsigned short sg_tablesize;	/* uas: SG-tabla meret */
};

struct usb_device {
	struct usb_device_descriptor descriptor;
	struct usb_host_config *config;       /* tömb: bNumConfigurations */
	struct usb_host_config *actconfig;
	int    devnum;
	enum usb_device_speed speed;
	enum usb_device_state state;
	char   product[64];
	char   manufacturer[64];
	char   serial[64];
	char   devpath[16];
	struct usb_bus *bus;
	struct usb_device *parent;
	int    quirks;
	struct device dev;
	void  *hcd_priv;                       /* uKernel: HCD fogantyú */
};

static inline struct usb_device *interface_to_usbdev(struct usb_interface *intf)
{ return intf->usb_dev; }
#define to_usb_interface(__dev) container_of(__dev, struct usb_interface, dev)
static inline void usb_set_intfdata(struct usb_interface *intf, void *data) { intf->intfdata = data; }
static inline void *usb_get_intfdata(struct usb_interface *intf) { return intf->intfdata; }
static inline struct usb_device *usb_get_dev(struct usb_device *d) { return d; }
static inline void usb_put_dev(struct usb_device *d) { (void)d; }
static inline struct usb_interface *usb_get_intf(struct usb_interface *i) { return i; }
static inline void usb_put_intf(struct usb_interface *i) { (void)i; }

/* ===== dynids (usb-serial bus.c) ===== */
struct usb_dynid {
	struct list_head node;
	struct usb_device_id id;
};
struct usb_dynids {
	spinlock_t lock;
	struct list_head list;
};

/* ===== driver ===== */
/* a valódi kernel usbdrv_wrap modellje, hogy a .drvwrap.driver.shutdown működjön */
struct bus_type;
struct attribute_group;
struct device_driver {
	const char *name;
	const struct bus_type *bus;
	struct module *owner;
	const struct attribute_group **groups;
	const struct attribute_group **dev_groups;
	bool suppress_bind_attrs;
	void (*shutdown)(struct device *dev);
	int  (*suspend)(struct device *dev, pm_message_t message);
	int  (*resume)(struct device *dev);
};
struct usbdrv_wrap { struct device_driver driver; };

struct usb_driver {
	const char *name;
	int  (*probe)(struct usb_interface *intf, const struct usb_device_id *id);
	void (*disconnect)(struct usb_interface *intf);
	int  (*suspend)(struct usb_interface *intf, pm_message_t message);
	int  (*resume)(struct usb_interface *intf);
	int  (*reset_resume)(struct usb_interface *intf);
	int  (*pre_reset)(struct usb_interface *intf);
	int  (*post_reset)(struct usb_interface *intf);
	void (*shutdown)(struct usb_interface *intf);  /* uas */
	const struct usb_device_id *id_table;
	struct usb_dynids dynids;
	struct usbdrv_wrap drvwrap;
	struct device_driver driver;     /* a >= 6.8 ág .driver.shutdown-jához */
	unsigned int no_dynamic_id:1;
	unsigned int supports_autosuspend:1;
	unsigned int disable_hub_initiated_lpm:1;
	unsigned int soft_unbind:1;
};

int  usb_register_driver(struct usb_driver *drv, struct module *owner, const char *name);
#define usb_register(drv) usb_register_driver(drv, THIS_MODULE, KBUILD_MODNAME)
void usb_deregister(struct usb_driver *drv);

/* module_usb_driver — a driver init/exit boilerplate generalasa */
#define module_usb_driver(__usb_driver) \
	static int __init __usb_driver##_init(void) \
	{ return usb_register(&(__usb_driver)); } \
	module_init(__usb_driver##_init); \
	static void __exit __usb_driver##_exit(void) \
	{ usb_deregister(&(__usb_driver)); } \
	module_exit(__usb_driver##_exit);

/* ===== usb-serial mag által igényelt kiegészítések ===== */
#include <linux/mutex.h>

extern struct mutex usb_dynids_lock;

#define to_usb_driver(d) container_of(d, struct usb_driver, driver)

const struct usb_device_id *usb_match_id(struct usb_interface *interface,
					 const struct usb_device_id *id);
int usb_match_one_id(struct usb_interface *interface,
		     const struct usb_device_id *id);
ssize_t usb_store_new_id(struct usb_dynids *dynids,
			 const struct usb_device_id *id_table,
			 struct device_driver *driver,
			 const char *buf, size_t count);
ssize_t usb_show_dynids(struct usb_dynids *dynids, char *buf);

int  usb_driver_claim_interface(struct usb_driver *driver,
				struct usb_interface *iface, void *data);
void usb_driver_release_interface(struct usb_driver *driver,
				  struct usb_interface *iface);

int usb_make_path(struct usb_device *dev, char *buf, size_t size);
int usb_translate_errors(int error_code);
int usb_disabled(void);

struct urb;
void usb_poison_urb(struct urb *urb);
void usb_unpoison_urb(struct urb *urb);

/* scatter-gather kerelem (usb-storage transport.c) */
struct scatterlist;
struct usb_sg_request {
	int		status;
	size_t		bytes;
	struct usb_device *dev;
	void		*priv;
};
int  usb_sg_init(struct usb_sg_request *io, struct usb_device *dev,
		 unsigned pipe, unsigned period, struct scatterlist *sg,
		 int nents, size_t length, gfp_t mem_flags);
void usb_sg_wait(struct usb_sg_request *io);
void usb_sg_cancel(struct usb_sg_request *io);

/* control-msg wrapperek (ch341.c) */
int usb_control_msg_send(struct usb_device *dev, __u8 endpoint, __u8 request,
			 __u8 requesttype, __u16 value, __u16 index,
			 const void *data, __u16 size, int timeout, gfp_t memflags);
int usb_control_msg_recv(struct usb_device *dev, __u8 endpoint, __u8 request,
			 __u8 requesttype, __u16 value, __u16 index,
			 void *data, __u16 size, int timeout, gfp_t memflags);

/* endpoint maxpacket + int-out helper (usb-serial.c) */
static inline int usb_endpoint_maxp(const struct usb_endpoint_descriptor *epd)
{ return epd->wMaxPacketSize & 0x07ff; }
static inline int usb_endpoint_is_int_out(const struct usb_endpoint_descriptor *e)
{ return usb_endpoint_xfer_int(e) && usb_endpoint_dir_out(e); }

/* usb_serial_driver.driver.owner / no_dynamic_id / needs_binding mezők kellenek;
 * a meglévő struct device_driver / usb_driver / usb_interface kiegészítése a
 * saját shim-headerben nem lehetséges, ezért lásd lent a wrap-mezőket. */

/* ===== pipe kódolás ===== */
/* pipe = (type & 3) | (ep_with_dir << 8); ep_with_dir bit7 = IN. */
#define __uk_pipe(type, ep, in) (((type) & 3) | ((((ep) & 0x0f) | ((in) ? 0x80 : 0)) << 8))
#define usb_sndctrlpipe(d, ep) __uk_pipe(USB_ENDPOINT_XFER_CONTROL, ep, 0)
#define usb_rcvctrlpipe(d, ep) __uk_pipe(USB_ENDPOINT_XFER_CONTROL, ep, 1)
#define usb_sndbulkpipe(d, ep) __uk_pipe(USB_ENDPOINT_XFER_BULK, ep, 0)
#define usb_rcvbulkpipe(d, ep) __uk_pipe(USB_ENDPOINT_XFER_BULK, ep, 1)
#define usb_sndintpipe(d, ep)  __uk_pipe(USB_ENDPOINT_XFER_INT, ep, 0)
#define usb_rcvintpipe(d, ep)  __uk_pipe(USB_ENDPOINT_XFER_INT, ep, 1)
#define usb_pipein(pipe)       (((pipe) >> 8) & 0x80)
#define usb_pipeout(pipe)      (!usb_pipein(pipe))
#define usb_pipeendpoint(pipe) (((pipe) >> 8) & 0x0f)
#define usb_pipetype(pipe)     ((pipe) & 3)

/* ===== URB ===== */
struct urb;
typedef void (*usb_complete_t)(struct urb *);

struct urb {
	struct usb_device *dev;
	unsigned int pipe;
	void   *transfer_buffer;
	dma_addr_t transfer_dma;
	u32     transfer_buffer_length;
	u32     actual_length;
	unsigned char *setup_packet;
	int     status;
	unsigned int transfer_flags;
	int     interval;
	usb_complete_t complete;
	void   *context;
	int     number_of_packets;
	int     error_count;
	void   *hcpriv;        /* uKernel: ukernel_async* */
	int     anchor_dummy;
	struct scatterlist *sg;
	int     num_sgs;
	unsigned int stream_id;	/* UAS bulk-stream */
	struct usb_anchor *anchor;
};

#define URB_NO_TRANSFER_DMA_MAP 0x0004
#define URB_ZERO_PACKET         0x0040
#define URB_FREE_BUFFER         0x0100
#define URB_SHORT_NOT_OK        0x0001

struct urb *usb_alloc_urb(int iso_packets, gfp_t mem_flags);
void usb_free_urb(struct urb *urb);
struct urb *usb_get_urb(struct urb *urb);
void usb_put_urb(struct urb *urb);
int  usb_maxpacket(struct usb_device *udev, int pipe);
int  usb_check_bulk_endpoints(const struct usb_interface *intf, const u8 *bEndpointAddress);
int  usb_check_int_endpoints(const struct usb_interface *intf, const u8 *bEndpointAddress);
int  usb_find_common_endpoints(struct usb_host_interface *alt,
		struct usb_endpoint_descriptor **bulk_in, struct usb_endpoint_descriptor **bulk_out,
		struct usb_endpoint_descriptor **int_in, struct usb_endpoint_descriptor **int_out);
int  usb_find_common_endpoints_reverse(struct usb_host_interface *alt,
		struct usb_endpoint_descriptor **bulk_in, struct usb_endpoint_descriptor **bulk_out,
		struct usb_endpoint_descriptor **int_in, struct usb_endpoint_descriptor **int_out);
int  usb_submit_urb(struct urb *urb, gfp_t mem_flags);
void usb_kill_urb(struct urb *urb);
int  usb_unlink_urb(struct urb *urb);

static inline void usb_fill_bulk_urb(struct urb *urb, struct usb_device *dev,
		unsigned int pipe, void *buf, int len, usb_complete_t complete, void *ctx)
{ urb->dev = dev; urb->pipe = pipe; urb->transfer_buffer = buf;
  urb->transfer_buffer_length = len; urb->complete = complete; urb->context = ctx; }

static inline void usb_fill_int_urb(struct urb *urb, struct usb_device *dev,
		unsigned int pipe, void *buf, int len, usb_complete_t complete, void *ctx, int interval)
{ usb_fill_bulk_urb(urb, dev, pipe, buf, len, complete, ctx); urb->interval = interval; }

static inline void usb_fill_control_urb(struct urb *urb, struct usb_device *dev,
		unsigned int pipe, unsigned char *setup, void *buf, int len, usb_complete_t complete, void *ctx)
{ usb_fill_bulk_urb(urb, dev, pipe, buf, len, complete, ctx); urb->setup_packet = setup; }

/* ===== szinkron I/O ===== */
int usb_control_msg(struct usb_device *dev, unsigned int pipe, __u8 request,
		__u8 requesttype, __u16 value, __u16 index, void *data, __u16 size, int timeout);
int usb_bulk_msg(struct usb_device *dev, unsigned int pipe, void *data, int len,
		int *actual_length, int timeout);
int usb_interrupt_msg(struct usb_device *dev, unsigned int pipe, void *data, int len,
		int *actual_length, int timeout);

int usb_set_interface(struct usb_device *dev, int iface, int alternate);
int usb_clear_halt(struct usb_device *dev, int pipe);

/* bulk-stream API (UAS) — userspace-ben nincs valodi stream-tamogatas. */
struct usb_host_endpoint *usb_pipe_endpoint(struct usb_device *dev, unsigned int pipe);
int usb_alloc_streams(struct usb_interface *intf, struct usb_host_endpoint **eps,
		      unsigned int num_eps, unsigned int num_streams, gfp_t mem_flags);
int usb_free_streams(struct usb_interface *intf, struct usb_host_endpoint **eps,
		     unsigned int num_eps, gfp_t mem_flags);
int usb_reset_device(struct usb_device *dev);
int usb_get_descriptor(struct usb_device *dev, unsigned char type, unsigned char index,
		void *buf, int size);
int usb_string(struct usb_device *dev, int index, char *buf, size_t size);

/* DMA-koherens pufferek (userspace-ben sima malloc) */
void *usb_alloc_coherent(struct usb_device *dev, size_t size, gfp_t mem_flags, dma_addr_t *dma);
void  usb_free_coherent(struct usb_device *dev, size_t size, void *addr, dma_addr_t dma);
#define usb_buffer_alloc(d, s, f, dma) usb_alloc_coherent(d, s, f, dma)
#define usb_buffer_free(d, s, a, dma)  usb_free_coherent(d, s, a, dma)

/* anchor — egyszerűsített no-op-szintű követés */
struct usb_anchor { struct list_head urb_list; };
static inline void init_usb_anchor(struct usb_anchor *a) { INIT_LIST_HEAD(&a->urb_list); }
static inline void usb_anchor_urb(struct urb *u, struct usb_anchor *a) { (void)u; (void)a; }
static inline void usb_unanchor_urb(struct urb *u) { (void)u; }
void usb_kill_anchored_urbs(struct usb_anchor *anchor);
void usb_poison_anchored_urbs(struct usb_anchor *anchor);
void usb_unpoison_anchored_urbs(struct usb_anchor *anchor);
struct urb *usb_get_from_anchor(struct usb_anchor *anchor);
int usb_anchor_empty(struct usb_anchor *anchor);
int usb_wait_anchor_empty_timeout(struct usb_anchor *anchor, unsigned int timeout);
void usb_scuttle_anchored_urbs(struct usb_anchor *anchor);

/* topológia-keresők */
struct usb_interface *usb_ifnum_to_if(const struct usb_device *dev, unsigned ifnum);
struct usb_host_interface *usb_altnum_to_altsetting(const struct usb_interface *intf,
						    unsigned int altnum);
struct usb_host_interface *usb_find_alt_setting(struct usb_host_config *config,
				unsigned int iface_num, unsigned int alt_num);

/* autosuspend — no-op */
static inline int  usb_autopm_get_interface(struct usb_interface *i) { (void)i; return 0; }
static inline void usb_autopm_put_interface(struct usb_interface *i) { (void)i; }
static inline void usb_enable_autosuspend(struct usb_device *d) { (void)d; }
static inline void usb_disable_autosuspend(struct usb_device *d) { (void)d; }

#endif

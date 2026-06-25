/* uKernel — HCD bridge interfész.
 *
 * A shim usb_core ezen az absztrakción keresztül beszél a valós (vagy mock)
 * USB eszközzel. Szándékosan NEM függ semmilyen linux/ típustól, hogy a
 * libusb backend tisztán fordítható legyen. */
#ifndef UKERNEL_HCD_H
#define UKERNEL_HCD_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* USB transzfer típusok (megegyeznek az USB spec endpoint típusaival). */
enum ukernel_xfer_type {
	UK_XFER_CONTROL = 0,
	UK_XFER_ISOC    = 1,
	UK_XFER_BULK    = 2,
	UK_XFER_INT     = 3,
};

/* Egy szinkron transzfer leírása. Az `ep` az endpoint címe a USB irány-bittel
 * (0x80 = IN). Control esetén a setup mezők érvényesek. */
struct ukernel_xfer {
	enum ukernel_xfer_type type;
	uint8_t  ep;           /* endpoint address (dir bit a 0x80-ban) */
	/* control setup csomag */
	uint8_t  bmRequestType;
	uint8_t  bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
	/* adatpuffer */
	void    *data;
	int      length;        /* kért hossz */
	int      actual_length; /* tényleges (kimenő) */
	unsigned timeout_ms;
};

/* Aszinkron transzfer (URB-ekhez). A completion az I/O befejeztével hívódik. */
struct ukernel_async;
typedef void (*ukernel_async_cb)(struct ukernel_async *a, void *user);

struct ukernel_async {
	struct ukernel_xfer xfer;
	ukernel_async_cb     complete;
	void                *user;
	int                  status;     /* 0 = ok, negatív = hiba (-errno) */
	void                *backend;     /* backend-privát (pl. libusb_transfer*) */
};

/* A backend opak fogantyúja egy megnyitott eszközhöz. */
struct ukernel_hcd;

struct ukernel_hcd_ops {
	const char *name;

	/* Eszköz megnyitása VID/PID alapján (0,0 = első elérhető).
	 * Visszaad egy fogantyút vagy NULL-t. */
	struct ukernel_hcd *(*open)(uint16_t vid, uint16_t pid);

	/* Nyers descriptorok lekérése. A puffer a backendé marad érvényes a
	 * close-ig; *len a hossz. */
	int (*get_device_descriptor)(struct ukernel_hcd *, const uint8_t **buf, int *len);
	int (*get_config_descriptor)(struct ukernel_hcd *, int index,
	                             const uint8_t **buf, int *len);

	/* Konfiguráció / interfész kiválasztás. */
	int (*set_configuration)(struct ukernel_hcd *, int config);
	int (*claim_interface)(struct ukernel_hcd *, int iface);
	int (*set_interface)(struct ukernel_hcd *, int iface, int alt);

	/* Szinkron transzferek. Visszaad 0-t vagy -errno-t; actual_length kitöltve. */
	int (*xfer)(struct ukernel_hcd *, struct ukernel_xfer *);

	/* Aszinkron transzfer beküldése / megszakítása. */
	int (*submit)(struct ukernel_hcd *, struct ukernel_async *);
	int (*cancel)(struct ukernel_hcd *, struct ukernel_async *);

	/* A backend eseményhurkának egy lépése (libusb_handle_events). A shim
	 * egy dedikált szálon hívja, ha submit-alapú I/O van. */
	int (*pump_events)(struct ukernel_hcd *, int timeout_ms);

	void (*close)(struct ukernel_hcd *);
};

/* A két beépített backend. */
const struct ukernel_hcd_ops *ukernel_hcd_libusb(void);
const struct ukernel_hcd_ops *ukernel_hcd_mock(void);

#ifdef __cplusplus
}
#endif
#endif /* UKERNEL_HCD_H */

/* uKernel hamis <linux/input.h> — minimalis input-reteg az onetouch.c-hez.
 * Userspace-ben nincs valodi input-alrendszer; a register/report no-op. */
#ifndef _UK_LINUX_INPUT_H
#define _UK_LINUX_INPUT_H

#include <linux/types.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/bitops.h>

#define EV_SYN		0x00
#define EV_KEY		0x01
#define EV_REL		0x02
#define EV_ABS		0x03

#define BUS_USB		0x03

#define KEY_PROG1	148

#define KEY_MAX		0x2ff
#define KEY_CNT		(KEY_MAX + 1)
#define EV_CNT		(0x1f + 1)

struct input_id {
	__u16 bustype;
	__u16 vendor;
	__u16 product;
	__u16 version;
};

struct input_dev {
	const char	*name;
	const char	*phys;
	struct input_id	id;
	unsigned long	evbit[BITS_TO_LONGS(EV_CNT)];
	unsigned long	keybit[BITS_TO_LONGS(KEY_CNT)];
	unsigned int	keycodemax;
	int		(*open)(struct input_dev *dev);
	void		(*close)(struct input_dev *dev);
	struct device	dev;
	void		*priv;
};

static inline struct input_dev *input_allocate_device(void)
{ return kzalloc(sizeof(struct input_dev), 0); }
static inline void input_free_device(struct input_dev *dev) { kfree(dev); }
static inline int input_register_device(struct input_dev *dev) { (void)dev; return 0; }
static inline void input_unregister_device(struct input_dev *dev) { kfree(dev); }
static inline void input_set_drvdata(struct input_dev *dev, void *data) { dev->priv = data; }
static inline void *input_get_drvdata(struct input_dev *dev) { return dev->priv; }
static inline void input_report_key(struct input_dev *dev, unsigned int code, int value)
{ (void)dev; (void)code; (void)value; }
static inline void input_sync(struct input_dev *dev) { (void)dev; }
static inline void input_set_capability(struct input_dev *dev, unsigned int type, unsigned int code)
{ (void)dev; (void)type; (void)code; }

#endif

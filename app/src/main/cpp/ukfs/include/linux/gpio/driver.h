/* uKernel shim — minimális GPIO-chip (cp210x/ftdi_sio GPIO-támogatás). A bedugott
 * UART-chipek GPIO-ját nem vezetjük ki; a regisztráció no-op (a driver enélkül is megy). */
#ifndef _UKNL_GPIO_DRIVER_H
#define _UKNL_GPIO_DRIVER_H
#include <linux/types.h>
#include <linux/device.h>
struct gpio_chip;
struct gpio_irq_chip { void *parent; };
struct gpio_chip {
	const char *label;
	struct device *parent;
	void *owner;
	int (*request)(struct gpio_chip *, unsigned int);
	void (*free)(struct gpio_chip *, unsigned int);
	int (*get_direction)(struct gpio_chip *, unsigned int);
	int (*direction_input)(struct gpio_chip *, unsigned int);
	int (*direction_output)(struct gpio_chip *, unsigned int, int);
	int (*get)(struct gpio_chip *, unsigned int);
	void (*set)(struct gpio_chip *, unsigned int, int);
	int (*set_config)(struct gpio_chip *, unsigned int, unsigned long);
	int base;
	u16 ngpio;
	const char *const *names;
	bool can_sleep;
};
static inline int gpiochip_add_data(struct gpio_chip *gc, void *data) { (void)gc; (void)data; return 0; }
static inline void gpiochip_remove(struct gpio_chip *gc) { (void)gc; }
static inline void *gpiochip_get_data(struct gpio_chip *gc) { (void)gc; return NULL; }
#define devm_gpiochip_add_data(dev, gc, data) gpiochip_add_data((gc), (data))
#endif

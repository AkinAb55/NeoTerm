/* uKernel hamis <linux/device.h> — minimál struct device + dev_* logok. */
#ifndef _UK_LINUX_DEVICE_H
#define _UK_LINUX_DEVICE_H

#include <linux/types.h>
#include <linux/kernel.h>

struct device_type { const char *name; void (*release)(struct device *dev); };
struct dev_pm_info { bool is_prepared; bool is_suspended; };

struct device_driver;
struct bus_type;
struct attribute_group;

struct kobject { const char *name; unsigned int state_in_sysfs:1; unsigned int state_initialized:1; };

struct device {
	struct kobject kobj;
	struct device *parent;
	const char    *init_name;
	void          *driver_data;   /* dev_get/set_drvdata */
	void          *platform_data;
	const struct device_type *type;
	struct device_driver *driver;
	const struct bus_type *bus;
	const struct attribute_group **groups;
	struct dev_pm_info power;
	struct fwnode_handle *fwnode;
	struct device_node *of_node;
	void         (*release)(struct device *);
};
struct fwnode_handle;
struct device_node;

/* ===== sysfs attribútumok (usb-serial bus.c / usb-serial.c) ===== */
struct kobject;
struct attribute { const char *name; umode_t mode; };
struct attribute_group {
	const char *name;
	umode_t (*is_visible)(struct kobject *, struct attribute *, int);
	struct attribute **attrs;
};

struct device_attribute {
	struct attribute attr;
	ssize_t (*show)(struct device *dev, struct device_attribute *attr, char *buf);
	ssize_t (*store)(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t count);
};
struct driver_attribute {
	struct attribute attr;
	ssize_t (*show)(struct device_driver *driver, char *buf);
	ssize_t (*store)(struct device_driver *driver, const char *buf, size_t count);
};

#define __ATTR_RO(_name) { .attr = { .name = #_name, .mode = 0444 } }
#define __ATTR_RW(_name) { .attr = { .name = #_name, .mode = 0644 } }

#define DEVICE_ATTR_RO(_name) \
	struct device_attribute dev_attr_##_name = { .attr = { .name = #_name, .mode = 0444 }, .show = _name##_show }
#define DEVICE_ATTR_RW(_name) \
	struct device_attribute dev_attr_##_name = { .attr = { .name = #_name, .mode = 0644 }, .show = _name##_show, .store = _name##_store }
#define DEVICE_ATTR_WO(_name) \
	struct device_attribute dev_attr_##_name = { .attr = { .name = #_name, .mode = 0200 }, .store = _name##_store }
#define DEVICE_ATTR(_name, _mode, _show, _store) \
	struct device_attribute dev_attr_##_name = { .attr = { .name = #_name, .mode = (_mode) }, .show = (_show), .store = (_store) }

/* sysfs attribútum-fájlok — userspace shimben no-op (a tényleges def serial_compat.c) */
int  device_create_file(struct device *dev, const struct device_attribute *attr);
void device_remove_file(struct device *dev, const struct device_attribute *attr);

/* devm_ resource-allokátorok — stub: sima k*alloc */
void *devm_kmalloc(struct device *dev, size_t size, gfp_t gfp);
void *devm_kzalloc(struct device *dev, size_t size, gfp_t gfp);
void *devm_kcalloc(struct device *dev, size_t n, size_t size, gfp_t gfp);
void  devm_kfree(struct device *dev, void *p);
#define DRIVER_ATTR_RO(_name) \
	struct driver_attribute driver_attr_##_name = { .attr = { .name = #_name, .mode = 0444 }, .show = _name##_show }
#define DRIVER_ATTR_RW(_name) \
	struct driver_attribute driver_attr_##_name = { .attr = { .name = #_name, .mode = 0644 }, .show = _name##_show, .store = _name##_store }

#define ATTRIBUTE_GROUPS(_name)						\
static const struct attribute_group _name##_group = {			\
	.attrs = _name##_attrs,						\
};									\
static const struct attribute_group *_name##_groups[] = {		\
	&_name##_group,							\
	NULL,								\
}

/* ===== bus_type ===== */
struct bus_type {
	const char *name;
	const struct attribute_group **drv_groups;
	int  (*match)(struct device *dev, const struct device_driver *drv);
	int  (*probe)(struct device *dev);
	void (*remove)(struct device *dev);
};

int  bus_register(const struct bus_type *bus);
void bus_unregister(const struct bus_type *bus);

int  driver_register(struct device_driver *drv);
void driver_unregister(struct device_driver *drv);
int  driver_attach(struct device_driver *drv);

void device_initialize(struct device *dev);
int  device_add(struct device *dev);
void device_del(struct device *dev);
int  device_register(struct device *dev);
void device_unregister(struct device *dev);
void get_device(struct device *dev);
void put_device(struct device *dev);
void device_enable_async_suspend(struct device *dev);
int  device_is_registered(struct device *dev);

#define kobj_to_dev(kobj) container_of((kobj), struct device, kobj)

static inline void *dev_get_drvdata(const struct device *d) { return d->driver_data; }
static inline void  dev_set_drvdata(struct device *d, void *data) { d->driver_data = data; }
const char *dev_name(const struct device *d);
int dev_set_name(struct device *d, const char *fmt, ...) __printf(2, 3);

/* dev_* logok printk-ra képezve (a device-előtag elhagyva az egyszerűségért) */
#define dev_emerg(dev, fmt, ...)  printk(KERN_EMERG   fmt, ##__VA_ARGS__)
#define dev_err(dev, fmt, ...)    printk(KERN_ERR     fmt, ##__VA_ARGS__)
#define dev_warn(dev, fmt, ...)   printk(KERN_WARNING fmt, ##__VA_ARGS__)
#define dev_notice(dev, fmt, ...) printk(KERN_NOTICE  fmt, ##__VA_ARGS__)
#define dev_info(dev, fmt, ...)   printk(KERN_INFO    fmt, ##__VA_ARGS__)
#define dev_dbg(dev, fmt, ...)    printk(KERN_DEBUG   fmt, ##__VA_ARGS__)
#define dev_printk(lvl, dev, fmt, ...) printk(fmt, ##__VA_ARGS__)

#endif
#define dev_warn_ratelimited(dev, fmt, ...) dev_warn((dev), fmt, ##__VA_ARGS__)

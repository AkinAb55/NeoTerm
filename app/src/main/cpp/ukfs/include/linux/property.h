#ifndef _UK_LINUX_PROPERTY_H
#define _UK_LINUX_PROPERTY_H
#include <linux/types.h>
struct fwnode_handle;
struct device;
static inline struct fwnode_handle *dev_fwnode(const struct device *dev) { (void)dev; return 0; }
static inline int device_property_read_u32(struct device *dev, const char *p, u32 *v) { (void)dev;(void)p;(void)v; return -1; }
static inline bool device_property_present(struct device *dev, const char *p) { (void)dev;(void)p; return false; }
static inline struct fwnode_handle *device_get_named_child_node(const struct device *dev, const char *name) { (void)dev;(void)name; return 0; }
static inline void fwnode_handle_put(struct fwnode_handle *fwnode) { (void)fwnode; }
#endif

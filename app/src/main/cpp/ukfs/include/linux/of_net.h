#ifndef _UK_LINUX_OF_NET_H
#define _UK_LINUX_OF_NET_H
#include <linux/types.h>
struct device_node; struct net_device;
static inline int of_get_mac_address(struct device_node *np, u8 *mac) { (void)np;(void)mac; return -1; }
static inline int device_get_mac_address(struct device *dev, char *addr) { (void)dev;(void)addr; return -1; }
#endif

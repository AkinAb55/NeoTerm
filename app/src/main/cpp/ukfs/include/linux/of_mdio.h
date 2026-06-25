#ifndef _UK_LINUX_OF_MDIO_H
#define _UK_LINUX_OF_MDIO_H
#include <linux/types.h>
#include <linux/phy.h>
struct device_node;
static inline int of_mdiobus_register(struct mii_bus *mdio, struct device_node *np) { (void)mdio;(void)np; return 0; }
static inline struct phy_device *of_phy_find_device(struct device_node *np) { (void)np; return 0; }
static inline struct phy_device *of_phy_connect(struct net_device *dev, struct device_node *np, void (*hndlr)(struct net_device *), u32 flags, phy_interface_t iface) { (void)dev;(void)np;(void)hndlr;(void)flags;(void)iface; return 0; }
#endif

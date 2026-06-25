#ifndef _UK_LINUX_MDIO_H
#define _UK_LINUX_MDIO_H
#include <linux/types.h>
#include <linux/mii.h>
#include <uapi/linux/mdio.h>
struct mdio_if_info {
	int prtad;
	u32 mmds;
	unsigned mode_support;
	struct net_device *dev;
	int (*mdio_read)(struct net_device *dev, int prtad, int devad, u16 addr);
	int (*mdio_write)(struct net_device *dev, int prtad, int devad, u16 addr, u16 val);
};
#define MDIO_PRTAD_NONE		(-1)
#define MDIO_DEVAD_NONE		(-1)
#define MDIO_SUPPORTS_C22_X	1
#define MDIO_SUPPORTS_C45_X	2
int mdio45_probe(struct mdio_if_info *mdio, int prtad);
int mdio_mii_ioctl(const struct mdio_if_info *mdio, struct mii_ioctl_data *mii_data, int cmd);
#endif

/* uKernel hamis <linux/phylink.h> — minimalis, az asix/ax88172a driverekhez
 * eleg. A tenyleges PHY-fuggvenyek a net_compat.c-ben stubok. */
#ifndef _UK_LINUX_PHYLINK_H
#define _UK_LINUX_PHYLINK_H
#include <linux/types.h>
#include <linux/phy.h>
#include <linux/ethtool.h>

enum phylink_op_type { PHYLINK_NETDEV = 0, PHYLINK_DEV };

#define MAC_SYM_PAUSE		0x01
#define MAC_ASYM_PAUSE		0x02
#define MAC_10HD		0x04
#define MAC_10FD		0x08
#define MAC_10			(MAC_10HD | MAC_10FD)
#define MAC_100HD		0x10
#define MAC_100FD		0x20
#define MAC_100			(MAC_100HD | MAC_100FD)
#define MAC_1000HD		0x40
#define MAC_1000FD		0x80
#define MAC_1000		(MAC_1000HD | MAC_1000FD)

struct phylink_config {
	struct device *dev;
	enum phylink_op_type type;
	bool mac_managed_pm;
	unsigned long mac_capabilities;
	DECLARE_BITMAP(supported_interfaces, 64);
};

struct phylink_link_state {
	DECLARE_BITMAP(advertising, __ETHTOOL_LINK_MODE_MASK_NBITS);
	DECLARE_BITMAP(lp_advertising, __ETHTOOL_LINK_MODE_MASK_NBITS);
	phy_interface_t interface;
	int speed; int duplex; int pause; int link; int an_complete;
};

struct phylink_mac_ops {
	void (*validate)(struct phylink_config *, unsigned long *, struct phylink_link_state *);
	void (*mac_config)(struct phylink_config *, unsigned int, const struct phylink_link_state *);
	void (*mac_link_up)(struct phylink_config *, struct phy_device *, unsigned int, phy_interface_t, int, int, bool, bool);
	void (*mac_link_down)(struct phylink_config *, unsigned int, phy_interface_t);
};

struct phylink;

struct phylink *phylink_create(struct phylink_config *cfg, void *fwnode,
			       phy_interface_t iface, const struct phylink_mac_ops *ops);
void phylink_destroy(struct phylink *pl);
int  phylink_connect_phy(struct phylink *pl, struct phy_device *phy);
void phylink_disconnect_phy(struct phylink *pl);
void phylink_start(struct phylink *pl);
void phylink_stop(struct phylink *pl);
void phylink_suspend(struct phylink *pl, bool mac_wol);
void phylink_resume(struct phylink *pl);
int  phylink_ethtool_get_pauseparam(struct phylink *pl, struct ethtool_pauseparam *pp);
int  phylink_ethtool_set_pauseparam(struct phylink *pl, struct ethtool_pauseparam *pp);
int  phylink_ethtool_ksettings_get(struct phylink *pl, struct ethtool_link_ksettings *cmd);
int  phylink_ethtool_ksettings_set(struct phylink *pl, const struct ethtool_link_ksettings *cmd);
int  phylink_mii_ioctl(struct phylink *pl, struct ifreq *ifr, int cmd);

static inline void phylink_set_pcs(struct phylink *pl, void *pcs) { (void)pl;(void)pcs; }

#endif

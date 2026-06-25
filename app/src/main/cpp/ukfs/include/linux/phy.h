#ifndef _UK_LINUX_PHY_H
#define _UK_LINUX_PHY_H
#include <linux/types.h>
#include <linux/mii.h>
#include <linux/mdio.h>
#include <linux/ethtool.h>
#include <linux/device.h>
struct phy_device; struct mii_bus; struct net_device;
#define PHY_INTERFACE_MODE_MII 1
#define PHY_INTERFACE_MODE_RMII 2
#define PHY_INTERFACE_MODE_RGMII 3
#define PHY_INTERFACE_MODE_INTERNAL 4
typedef int phy_interface_t;
#define PHY_POLL -1
#define PHY_MAX_ADDR 32
#define PHY_ID_SIZE 32
#define PHY_ID_FMT "%s:%02x"
int phy_ethtool_nway_reset(struct net_device *ndev);
int genphy_resume(struct phy_device *phydev);
int genphy_suspend(struct phy_device *phydev);
int phy_ethtool_get_link_ksettings(struct net_device *ndev, struct ethtool_link_ksettings *cmd);
int phy_ethtool_set_link_ksettings(struct net_device *ndev, const struct ethtool_link_ksettings *cmd);
int phy_mii_ioctl(struct phy_device *phydev, struct ifreq *ifr, int cmd);
int phy_do_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd);
int phy_do_ioctl_running(struct net_device *dev, struct ifreq *ifr, int cmd);
struct mdio_device {
	struct device dev;
	struct mii_bus *bus;
	int addr;
};

struct phy_device {
	struct mdio_device mdio;
	struct mii_bus *mdio_bus;
	int addr; int phy_id; int speed; int duplex; int link; int autoneg;
	bool mac_managed_pm;
	bool is_internal;
	bool is_pseudo_fixed_link;
	bool suspended;
	int interface;
	int irq;
	struct net_device *attached_dev;
	DECLARE_BITMAP(supported, __ETHTOOL_LINK_MODE_MASK_NBITS);
	DECLARE_BITMAP(advertising, __ETHTOOL_LINK_MODE_MASK_NBITS);
};
struct mii_bus {
	const char *name; char id[32]; void *priv;
	int (*read)(struct mii_bus *bus, int phy_id, int regnum);
	int (*write)(struct mii_bus *bus, int phy_id, int regnum, u16 val);
	int (*reset)(struct mii_bus *bus);
	struct device *parent; int phy_mask;
	struct device dev;
	void *irq;
};
#define MII_BUS_ID_SIZE 61
static inline struct mii_bus *mdiobus_alloc(void) { return 0; }
static inline int mdiobus_register(struct mii_bus *bus) { (void)bus; return 0; }
static inline void mdiobus_unregister(struct mii_bus *bus) { (void)bus; }
static inline void mdiobus_free(struct mii_bus *bus) { (void)bus; }
static inline int mdiobus_read(struct mii_bus *bus, int addr, int regnum) { (void)bus;(void)addr;(void)regnum; return 0; }
static inline int mdiobus_write(struct mii_bus *bus, int addr, int regnum, u16 val) { (void)bus;(void)addr;(void)regnum;(void)val; return 0; }
struct phy_device *phy_connect(struct net_device *dev, const char *bus_id, void (*handler)(struct net_device *), phy_interface_t iface);
struct phy_device *mdiobus_get_phy(struct mii_bus *bus, int addr);
struct phy_device *phy_find_first(struct mii_bus *bus);
int phy_connect_direct(struct net_device *dev, struct phy_device *phydev, void (*handler)(struct net_device *), phy_interface_t iface);
void phy_disconnect(struct phy_device *phydev);
void phy_print_status(struct phy_device *phydev);
int  genphy_read_status(struct phy_device *phydev);
struct mii_bus *devm_mdiobus_alloc(struct device *dev);
int  devm_mdiobus_register(struct device *dev, struct mii_bus *bus);
static inline void phy_start(struct phy_device *p){ (void)p; }
static inline void phy_stop(struct phy_device *p){ (void)p; }
static inline int phy_start_aneg(struct phy_device *p){ (void)p; return 0; }
#endif

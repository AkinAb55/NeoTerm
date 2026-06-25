#ifndef _UK_UAPI_LINUX_MDIO_H
#define _UK_UAPI_LINUX_MDIO_H
#include <linux/types.h>
#define MDIO_MMD_PCS		3
#define MDIO_MMD_AN		7
#define MDIO_MMD_VEND2		31
#define MDIO_AN_EEE_ADV		60
#define MDIO_AN_EEE_LPABLE	61
#define MDIO_EEE_100TX		0x0002
#define MDIO_EEE_1000T		0x0004
#define MDIO_PCS_EEE_ABLE	20
#define MDIO_PCS_EEE_ABLE2	21
#define MDIO_AN_EEE_ADV2	62
#define MDIO_AN_EEE_LPABLE2	64
#define MDIO_EEE_2_5GT		0x0001
#define MDIO_EEE_5GT		0x0002
#define MDIO_EEE_10GT		0x0008
#define MDIO_MMD_PMAPMD		1
#define MDIO_MMD_PHYXS		4
#define MDIO_MMD_DTEXS		5
#define MDIO_MMD_TC		6
#define MDIO_MMD_C22EXT		29
#define MDIO_AN_10GBT_CTRL	32
#define MDIO_AN_10GBT_CTRL_ADV2_5G	0x0080
#define MDIO_AN_10GBT_CTRL_ADV5G	0x0100
#define MDIO_AN_10GBT_CTRL_ADV10G	0x1000
#define MDIO_AN_10GBT_STAT	33
#define MDIO_AN_10GBT_STAT_LP2_5G	0x0020
#define MDIO_AN_10GBT_STAT_LP5G		0x0040
#define MDIO_AN_10GBT_STAT_LP10G	0x0800
#define MDIO_CTRL1		0x00
#define MDIO_STAT1		0x01
#define MDIO_AN_ADVERTISE	16
#define MII_ADDR_C45		(1u<<30)
#define MII_DEVADDR_C45_SHIFT	16
#define MII_REGADDR_C45_MASK	0xffff
static inline __u32 mdiobus_c45_addr(int devad, __u16 regnum) { return MII_ADDR_C45 | ((__u32)devad << MII_DEVADDR_C45_SHIFT) | regnum; }
static inline int mdio_phy_id_is_c45(int phy_id) { (void)phy_id; return 0; }
#endif

/* uKernel hamis <linux/ethtool.h> — minimalis, de a net/usb driverekhez
 * eleg gazdag: link_ksettings + link-mode maszk + ethtool_ops + uapi adat-
 * strukturak. */
#ifndef _UK_LINUX_ETHTOOL_H
#define _UK_LINUX_ETHTOOL_H

#include <linux/types.h>
#include <linux/bitops.h>
#include <linux/if.h>

struct net_device;
struct module;

#define ETHTOOL_FWVERS_LEN	32
#define ETHTOOL_BUSINFO_LEN	32
#define ETHTOOL_EROMVERS_LEN	32
#define ETH_GSTRING_LEN		32

/* SPEED/DUPLEX/PORT enumok */
#define SPEED_10		10
#define SPEED_100		100
#define SPEED_1000		1000
#define SPEED_2500		2500
#define SPEED_5000		5000
#define SPEED_10000		10000
#define SPEED_UNKNOWN		-1

#define DUPLEX_HALF		0x00
#define DUPLEX_FULL		0x01
#define DUPLEX_UNKNOWN		0xff

#define PORT_TP			0x00
#define PORT_AUI		0x01
#define PORT_MII		0x02
#define PORT_FIBRE		0x03
#define PORT_BNC		0x04
#define PORT_DA			0x05
#define PORT_NONE		0xef
#define PORT_OTHER		0xff

#define XCVR_INTERNAL		0x00
#define XCVR_EXTERNAL		0x01

#define AUTONEG_DISABLE		0x00
#define AUTONEG_ENABLE		0x01

#define MDIO_SUPPORTS_C22	1
#define MDIO_SUPPORTS_C45	2

#define WAKE_PHY		(1 << 0)
#define WAKE_UCAST		(1 << 1)
#define WAKE_MCAST		(1 << 2)
#define WAKE_BCAST		(1 << 3)
#define WAKE_ARP		(1 << 4)
#define WAKE_MAGIC		(1 << 5)
#define WAKE_MAGICSECURE	(1 << 6)

/* link-mode bit indexek (csak az itt hasznaltak) */
enum ethtool_link_mode_bit_indices {
	ETHTOOL_LINK_MODE_10baseT_Half_BIT	= 0,
	ETHTOOL_LINK_MODE_10baseT_Full_BIT	= 1,
	ETHTOOL_LINK_MODE_100baseT_Half_BIT	= 2,
	ETHTOOL_LINK_MODE_100baseT_Full_BIT	= 3,
	ETHTOOL_LINK_MODE_1000baseT_Half_BIT	= 4,
	ETHTOOL_LINK_MODE_1000baseT_Full_BIT	= 5,
	ETHTOOL_LINK_MODE_Autoneg_BIT		= 6,
	ETHTOOL_LINK_MODE_TP_BIT		= 7,
	ETHTOOL_LINK_MODE_AUI_BIT		= 8,
	ETHTOOL_LINK_MODE_MII_BIT		= 9,
	ETHTOOL_LINK_MODE_FIBRE_BIT		= 10,
	ETHTOOL_LINK_MODE_BNC_BIT		= 11,
	ETHTOOL_LINK_MODE_Pause_BIT		= 12,
	ETHTOOL_LINK_MODE_Asym_Pause_BIT	= 13,
	ETHTOOL_LINK_MODE_2500baseT_Full_BIT	= 14,
	ETHTOOL_LINK_MODE_5000baseT_Full_BIT	= 15,
	__ETHTOOL_LINK_MODE_LAST		= 99,
};

#define __ETHTOOL_LINK_MODE_MASK_NBITS	(__ETHTOOL_LINK_MODE_LAST + 1)
#define __ETHTOOL_DECLARE_LINK_MODE_MASK(name)		\
	DECLARE_BITMAP(name, __ETHTOOL_LINK_MODE_MASK_NBITS)
#define ETHTOOL_DECLARE_LINK_MODE_MASK(name)		\
	DECLARE_BITMAP(name, __ETHTOOL_LINK_MODE_MASK_NBITS)

struct ethtool_link_settings {
	__u32	cmd;
	__u32	speed;
	__u8	duplex;
	__u8	port;
	__u8	phy_address;
	__u8	autoneg;
	__u8	mdio_support;
	__u8	eth_tp_mdix;
	__u8	eth_tp_mdix_ctrl;
	__s8	link_mode_masks_nwords;
	__u8	transceiver;
	__u8	master_slave_cfg;
	__u8	master_slave_state;
	__u8	rate_matching;
	__u32	reserved[7];
};

struct ethtool_link_ksettings {
	struct ethtool_link_settings base;
	struct {
		__ETHTOOL_DECLARE_LINK_MODE_MASK(supported);
		__ETHTOOL_DECLARE_LINK_MODE_MASK(advertising);
		__ETHTOOL_DECLARE_LINK_MODE_MASK(lp_advertising);
	} link_modes;
	__u32	lanes;
};

#define ethtool_link_ksettings_zero_link_mode(ptr, name)		\
	bitmap_zero((ptr)->link_modes.name, __ETHTOOL_LINK_MODE_MASK_NBITS)

#define ethtool_link_ksettings_add_link_mode(ptr, name, mode)		\
	__set_bit(ETHTOOL_LINK_MODE_ ## mode ## _BIT,			\
		  (ptr)->link_modes.name)

#define ethtool_link_ksettings_del_link_mode(ptr, name, mode)		\
	__clear_bit(ETHTOOL_LINK_MODE_ ## mode ## _BIT,			\
		    (ptr)->link_modes.name)

#define ethtool_link_ksettings_test_link_mode(ptr, name, mode)		\
	test_bit(ETHTOOL_LINK_MODE_ ## mode ## _BIT,			\
		 (ptr)->link_modes.name)

/* legacy ethtool_cmd */
struct ethtool_cmd {
	__u32	cmd;
	__u32	supported;
	__u32	advertising;
	__u16	speed;
	__u8	duplex;
	__u8	port;
	__u8	phy_address;
	__u8	transceiver;
	__u8	autoneg;
	__u8	mdio_support;
	__u32	maxtxpkt;
	__u32	maxrxpkt;
	__u16	speed_hi;
	__u8	eth_tp_mdix;
	__u8	eth_tp_mdix_ctrl;
	__u32	lp_advertising;
	__u32	reserved[2];
};

static inline __u32 ethtool_cmd_speed(const struct ethtool_cmd *ep)
{
	return (ep->speed_hi << 16) | ep->speed;
}

static inline void ethtool_cmd_speed_set(struct ethtool_cmd *ep, __u32 speed)
{
	ep->speed = (__u16)speed;
	ep->speed_hi = (__u16)(speed >> 16);
}

struct ethtool_drvinfo {
	__u32	cmd;
	char	driver[32];
	char	version[32];
	char	fw_version[ETHTOOL_FWVERS_LEN];
	char	bus_info[ETHTOOL_BUSINFO_LEN];
	char	erom_version[ETHTOOL_EROMVERS_LEN];
	char	reserved2[12];
	__u32	n_priv_flags;
	__u32	n_stats;
	__u32	testinfo_len;
	__u32	eedump_len;
	__u32	regdump_len;
};

struct ethtool_wolinfo {
	__u32	cmd;
	__u32	supported;
	__u32	wolopts;
	__u8	sopass[6];
};

struct ethtool_regs {
	__u32	cmd;
	__u32	version;
	__u32	len;
	__u8	data[];
};

struct ethtool_eeprom {
	__u32	cmd;
	__u32	magic;
	__u32	offset;
	__u32	len;
	__u8	data[];
};

struct ethtool_coalesce {
	__u32	cmd;
	__u32	rx_coalesce_usecs;
	__u32	rx_max_coalesced_frames;
	__u32	tx_coalesce_usecs;
	__u32	tx_max_coalesced_frames;
	__u32	reserved[16];
};

struct ethtool_ringparam {
	__u32	cmd;
	__u32	rx_max_pending;
	__u32	rx_mini_max_pending;
	__u32	rx_jumbo_max_pending;
	__u32	tx_max_pending;
	__u32	rx_pending;
	__u32	rx_mini_pending;
	__u32	rx_jumbo_pending;
	__u32	tx_pending;
};

struct ethtool_pauseparam {
	__u32	cmd;
	__u32	autoneg;
	__u32	rx_pause;
	__u32	tx_pause;
};

struct ethtool_stats {
	__u32	cmd;
	__u32	n_stats;
	__u64	data[];
};

struct ethtool_ts_info;
struct ethtool_test { __u32 cmd; __u32 flags; __u32 reserved; __u32 len; __u64 data[]; };
struct ethtool_eee;
struct ethtool_keee;
struct ethtool_tunable { __u32 cmd; __u32 id; __u32 type_id; __u32 len; void *data[]; };
#define ETHTOOL_RX_COPYBREAK	1
#define ETHTOOL_TX_COPYBREAK	2
#define ETHTOOL_PFC_PREVENTION_TOUT 3
enum ethtool_tunable_id { ETHTOOL_ID_UNSPEC, ETHTOOL_RX_COPYBREAK_ID };
struct ethtool_rxnfc;
struct ethtool_kernel_coalesce;
struct kernel_ethtool_coalesce;
struct netlink_ext_ack;
struct kernel_ethtool_ringparam;

struct ethtool_ops {
	int	(*get_link_ksettings)(struct net_device *,
				      struct ethtool_link_ksettings *);
	int	(*set_link_ksettings)(struct net_device *,
				      const struct ethtool_link_ksettings *);
	void	(*get_drvinfo)(struct net_device *, struct ethtool_drvinfo *);
	int	(*get_regs_len)(struct net_device *);
	void	(*get_regs)(struct net_device *, struct ethtool_regs *, void *);
	void	(*get_wol)(struct net_device *, struct ethtool_wolinfo *);
	int	(*set_wol)(struct net_device *, struct ethtool_wolinfo *);
	u32	(*get_msglevel)(struct net_device *);
	void	(*set_msglevel)(struct net_device *, u32);
	int	(*nway_reset)(struct net_device *);
	u32	(*get_link)(struct net_device *);
	int	(*get_eeprom_len)(struct net_device *);
	int	(*get_eeprom)(struct net_device *, struct ethtool_eeprom *, u8 *);
	int	(*set_eeprom)(struct net_device *, struct ethtool_eeprom *, u8 *);
	int	(*get_coalesce)(struct net_device *, struct ethtool_coalesce *,
				struct kernel_ethtool_coalesce *,
				struct netlink_ext_ack *);
	int	(*set_coalesce)(struct net_device *, struct ethtool_coalesce *,
				struct kernel_ethtool_coalesce *,
				struct netlink_ext_ack *);
	void	(*get_ringparam)(struct net_device *, struct ethtool_ringparam *,
				 struct kernel_ethtool_ringparam *,
				 struct netlink_ext_ack *);
	int	(*set_ringparam)(struct net_device *, struct ethtool_ringparam *,
				 struct kernel_ethtool_ringparam *,
				 struct netlink_ext_ack *);
	void	(*get_pauseparam)(struct net_device *, struct ethtool_pauseparam *);
	int	(*set_pauseparam)(struct net_device *, struct ethtool_pauseparam *);
	void	(*self_test)(struct net_device *, struct ethtool_test *, u64 *);
	void	(*get_strings)(struct net_device *, u32 stringset, u8 *);
	void	(*get_ethtool_stats)(struct net_device *,
				     struct ethtool_stats *, u64 *);
	int	(*get_sset_count)(struct net_device *, int);
	int	(*get_ts_info)(struct net_device *, struct ethtool_ts_info *);
	int	(*get_tunable)(struct net_device *,
			       const struct ethtool_tunable *, void *);
	int	(*set_tunable)(struct net_device *,
			       const struct ethtool_tunable *, const void *);
	int	(*get_eee)(struct net_device *, struct ethtool_keee *);
	int	(*set_eee)(struct net_device *, struct ethtool_keee *);
	int	(*begin)(struct net_device *);
	void	(*complete)(struct net_device *);
	int	(*get_sset_count_ext)(struct net_device *);
	u32	supported_coalesce_params;
	u32	cap_link_lanes_supported;
};

#define ETHTOOL_COALESCE_USECS			(1 << 0)
#define ETHTOOL_COALESCE_RX_USECS		(1 << 1)
#define ETHTOOL_COALESCE_TX_USECS		(1 << 2)
#define ETHTOOL_COALESCE_MAX_FRAMES		(1 << 3)
#define ETHTOOL_COALESCE_RX_MAX_FRAMES		(1 << 4)
#define ETHTOOL_COALESCE_TX_MAX_FRAMES		(1 << 5)

/* helper prototipusok (net_compat.c-ben stubolva) */
u32  ethtool_op_get_link(struct net_device *dev);
int  ethtool_op_get_ts_info(struct net_device *dev, struct ethtool_ts_info *eti);
int  __ethtool_get_link_ksettings(struct net_device *dev,
				  struct ethtool_link_ksettings *cmd);

/* ethtool_ts_info pollozas miatti minimalis def */
struct ethtool_ts_info {
	__u32	cmd;
	__u32	so_timestamping;
	__s32	phc_index;
	__u32	tx_types;
	__u32	tx_reserved[3];
	__u32	rx_filters;
	__u32	rx_reserved[3];
};

struct ethtool_eee {
	__u32	cmd;
	__u32	supported;
	__u32	advertised;
	__u32	lp_advertised;
	__u32	eee_active;
	__u32	eee_enabled;
	__u32	tx_lpi_enabled;
	__u32	tx_lpi_timer;
	__u32	reserved[2];
};

struct ethtool_keee {
	__ETHTOOL_DECLARE_LINK_MODE_MASK(supported);
	__ETHTOOL_DECLARE_LINK_MODE_MASK(advertised);
	__ETHTOOL_DECLARE_LINK_MODE_MASK(lp_advertised);
	u32	tx_lpi_timer;
	bool	tx_lpi_enabled;
	bool	eee_active;
	bool	eee_enabled;
};

/* legacy SUPPORTED_ / ADVERTISED_ bitmaszkok (32-bites) */
#define SUPPORTED_10baseT_Half		(1 << 0)
#define SUPPORTED_10baseT_Full		(1 << 1)
#define SUPPORTED_100baseT_Half		(1 << 2)
#define SUPPORTED_100baseT_Full		(1 << 3)
#define SUPPORTED_1000baseT_Half	(1 << 4)
#define SUPPORTED_1000baseT_Full	(1 << 5)
#define SUPPORTED_Autoneg		(1 << 6)
#define SUPPORTED_TP			(1 << 7)
#define SUPPORTED_AUI			(1 << 8)
#define SUPPORTED_MII			(1 << 9)
#define SUPPORTED_FIBRE			(1 << 10)
#define SUPPORTED_BNC			(1 << 11)
#define SUPPORTED_Pause			(1 << 13)
#define SUPPORTED_Asym_Pause		(1 << 14)
#define ADVERTISED_10baseT_Half		(1 << 0)
#define ADVERTISED_10baseT_Full		(1 << 1)
#define ADVERTISED_100baseT_Half	(1 << 2)
#define ADVERTISED_100baseT_Full	(1 << 3)
#define ADVERTISED_1000baseT_Half	(1 << 4)
#define ADVERTISED_1000baseT_Full	(1 << 5)
#define ADVERTISED_Autoneg		(1 << 6)
#define ADVERTISED_TP			(1 << 7)
#define ADVERTISED_AUI			(1 << 8)
#define ADVERTISED_MII			(1 << 9)
#define ADVERTISED_FIBRE		(1 << 10)
#define ADVERTISED_BNC			(1 << 11)
#define ADVERTISED_Pause		(1 << 13)
#define ADVERTISED_Asym_Pause		(1 << 14)

#define ETHTOOL_GSET		0x00000001
#define ETHTOOL_SSET		0x00000002
#define ETHTOOL_GDRVINFO	0x00000003
#define ETHTOOL_GLINK		0x0000000a
#define ETH_SS_TEST		0
#define ETH_SS_STATS		1
#define ETH_SS_PRIV_FLAGS	2

#endif

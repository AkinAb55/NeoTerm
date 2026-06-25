/* uKernel hamis <linux/netdevice.h> — minimál net_device. */
#ifndef _UK_LINUX_NETDEVICE_H
#define _UK_LINUX_NETDEVICE_H

#include <linux/types.h>
#include <linux/list.h>
#include <linux/device.h>
#include <linux/skbuff.h>
/* A net-usb build (hso) az ETH_P es ARPHRD konstansokat a netdevice.h-n at
 * varja. A rtl8812au-build (UKERNEL_DRIVER_BUILD) NEM kapja meg, mert a sajat
 * byteorder/generic.h-ja a mi konstans htons-unkat undef-eli. */
#ifndef UKERNEL_DRIVER_BUILD
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#endif
#include <linux/socket.h>      /* struct sockaddr (ndo_set_mac_address) */
#include <linux/timer.h>       /* a driverek a netdevice-en át kapják (mint a valódi kernelben) */
#include <linux/workqueue.h>
#include <linux/wait.h>

#define IFNAMSIZ 16
#define IFF_UP        0x1
#define IFF_BROADCAST 0x2
#define IFF_RUNNING   0x40
#define IFF_MULTICAST 0x1000
#define IFF_PROMISC   0x100
#define IFF_ALLMULTI  0x200
#define IFF_NOARP     0x80
#define IFF_POINTOPOINT 0x10
#define IFF_LOOPBACK  0x8
#define IFF_DEBUG     0x4
#define IFF_AUTOMEDIA 0x4000

typedef int netdev_tx_t;
#define NETDEV_TX_OK   0
#define NETDEV_TX_BUSY 0x10
#define NET_XMIT_SUCCESS 0
#define NET_XMIT_DROP    1
#define NET_XMIT_CN      2
#define netdev_features_t u64

struct net_device;
struct wireless_dev;

struct net_device_stats {
	unsigned long rx_packets, tx_packets, rx_bytes, tx_bytes;
	unsigned long rx_errors, tx_errors, rx_dropped, tx_dropped;
	unsigned long multicast, collisions;
	unsigned long rx_length_errors, rx_over_errors, rx_crc_errors, rx_frame_errors;
	unsigned long rx_fifo_errors, rx_missed_errors;
	unsigned long tx_aborted_errors, tx_carrier_errors, tx_fifo_errors;
	unsigned long tx_heartbeat_errors, tx_window_errors;
	unsigned long rx_compressed, tx_compressed;
	unsigned long rx_nohandler;
};

struct ifreq;
struct ethtool_ops;
struct header_ops;

#define NET_ADDR_PERM		0
#define NET_ADDR_RANDOM		1
#define NET_ADDR_STOLEN		2
#define NET_ADDR_SET		3
#define NET_NAME_UNKNOWN	0
#define NET_NAME_ENUM		1
#define NET_NAME_PREDICTABLE	2
#define NET_NAME_USER		3

struct pcpu_sw_netstats {
	u64 rx_packets, rx_bytes, tx_packets, tx_bytes;
	struct u64_stats_sync syncp;
};

enum netdev_stat_type {
	NETDEV_PCPU_STAT_NONE,
	NETDEV_PCPU_STAT_LSTATS,
	NETDEV_PCPU_STAT_TSTATS,
	NETDEV_PCPU_STAT_DSTATS,
};

struct net_device_ops {
	int  (*ndo_init)(struct net_device *dev);
	void (*ndo_uninit)(struct net_device *dev);
	int  (*ndo_open)(struct net_device *dev);
	int  (*ndo_stop)(struct net_device *dev);
	netdev_tx_t (*ndo_start_xmit)(struct sk_buff *skb, struct net_device *dev);
	u16  (*ndo_select_queue)(struct net_device *dev, struct sk_buff *skb, struct net_device *sb_dev);
	int  (*ndo_do_ioctl)(struct net_device *dev, void *ifr, int cmd);
	int  (*ndo_eth_ioctl)(struct net_device *dev, struct ifreq *ifr, int cmd);
	int  (*ndo_siocdevprivate)(struct net_device *dev, struct ifreq *ifr,
				   void *data, int cmd);
	void (*ndo_set_rx_mode)(struct net_device *dev);
	int  (*ndo_set_mac_address)(struct net_device *dev, void *addr);
	int  (*ndo_validate_addr)(struct net_device *dev);
	struct net_device_stats *(*ndo_get_stats)(struct net_device *dev);
	void (*ndo_get_stats64)(struct net_device *dev, void *storage);
	int  (*ndo_change_mtu)(struct net_device *dev, int new_mtu);
	void (*ndo_tx_timeout)(struct net_device *dev, unsigned int txqueue);
	int  (*ndo_set_features)(struct net_device *dev, netdev_features_t features);
	netdev_features_t (*ndo_fix_features)(struct net_device *dev,
					      netdev_features_t features);
	int  (*ndo_add_slave)(struct net_device *dev, struct net_device *slave);
	int  (*ndo_vlan_rx_add_vid)(struct net_device *dev, __be16 proto, u16 vid);
	int  (*ndo_vlan_rx_kill_vid)(struct net_device *dev, __be16 proto, u16 vid);
	netdev_features_t (*ndo_features_check)(struct sk_buff *skb, struct net_device *dev,
				   netdev_features_t features);
};

struct net_device {
	char   name[IFNAMSIZ];
	const struct net_device_ops *netdev_ops;
	const struct ethtool_ops *ethtool_ops;
	const struct iw_handler_def *wireless_handlers;
	struct wireless_dev *ieee80211_ptr;
	void  *ml_priv;
	unsigned char dev_addr[6];
	unsigned int  flags;
	unsigned int  priv_flags;
	unsigned int  mtu, min_mtu, max_mtu;
	u64           features, hw_features, hw_enc_features, vlan_features;
	u64           wanted_features;
	unsigned short type;
	unsigned char addr_len;
	unsigned char addr_assign_type;
	const struct header_ops *header_ops;
	unsigned short hard_header_len;
	unsigned short needed_headroom;
	unsigned short needed_tailroom;
	int           watchdog_timeo;
	int           msg_enable;
	unsigned int  tx_queue_len;
	unsigned int  num_tx_queues, num_rx_queues, real_num_tx_queues;
	unsigned int  gso_max_size;
	unsigned short gso_max_segs;
	struct net_device_stats stats;
	void          *tstats;          /* pcpu_sw_netstats (egyszerusitett) */
	enum netdev_stat_type pcpu_stat_type;
	struct device dev;
	bool  needs_free_netdev;
	void (*priv_destructor)(struct net_device *dev);
	unsigned char perm_addr[6];
	unsigned char broadcast[6];
	struct phy_device *phydev;
	const struct attribute_group *sysfs_groups[4];
	struct {
		struct list_head upper;
		struct list_head lower;
	} adj_list;
	void  *priv;        /* netdev_priv */
};
struct phy_device;

#define IFF_TX_SKB_SHARING	0x10000
#define IFF_UNICAST_FLT		0x20000
#define IFF_LIVE_ADDR_CHANGE	0x40000
#define IFF_NO_QUEUE		0x80000

#define ETH_MAX_MTU		0xFFFFU
#define ETH_MIN_MTU		68
#define LL_MAX_HEADER		32
#define MAX_HEADER		LL_MAX_HEADER

#define NETIF_F_SG       (1ULL << 0)
#define NETIF_F_IP_CSUM  (1ULL << 1)
#define NETIF_F_TSO      (1ULL << 2)
#define NETIF_F_GSO      (1ULL << 3)
#define NETIF_F_HW_CSUM  (1ULL << 4)
#define NETIF_F_IP_CSUM_BIT 1
#define NETIF_F_IPV6_CSUM (1ULL << 13)
#define NETIF_F_RXCSUM   (1ULL << 5)
#define NETIF_F_TSO6     (1ULL << 6)
#define NETIF_F_GRO      (1ULL << 7)
#define NETIF_F_HIGHDMA  (1ULL << 8)
#define NETIF_F_FRAGLIST (1ULL << 9)
#define NETIF_F_HW_VLAN_CTAG_TX (1ULL << 10)
#define NETIF_F_HW_VLAN_CTAG_RX (1ULL << 11)
#define NETIF_F_VLAN_CHALLENGED (1ULL << 12)
#define NETIF_F_HW_VLAN_CTAG_FILTER (1ULL << 14)
#define NETIF_F_TSO_ECN  (1ULL << 15)
#define NETIF_F_LLTX     (1ULL << 16)
#define NETIF_F_NETNS_LOCAL (1ULL << 17)
#define NETIF_F_GSO_UDP_L4 (1ULL << 18)
#define NETIF_F_CSUM_MASK (NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM | NETIF_F_HW_CSUM)
#define NETIF_F_GSO_MASK  (NETIF_F_TSO | NETIF_F_TSO6 | NETIF_F_GSO | NETIF_F_GSO_UDP_L4)
#define NETIF_F_ALL_TSO   (NETIF_F_TSO | NETIF_F_TSO6 | NETIF_F_TSO_ECN)

/* netdev notifier események */
#define NETDEV_UP         0x0001
#define NETDEV_DOWN       0x0002
#define NETDEV_CHANGE     0x0004
#define NETDEV_CHANGENAME 0x000A
#define NETDEV_GOING_DOWN 0x0009
#define NETDEV_PRE_TYPE_CHANGE 0x000E
#define NETDEV_POST_TYPE_CHANGE 0x000F
#define NETDEV_REGISTER   0x0005
#define NETDEV_UNREGISTER 0x0006
#define NETDEV_CHANGEMTU  0x0007
#define NETDEV_CHANGEADDR 0x0008

struct net_device *dev_get_by_name(void *net, const char *name);
struct net { struct proc_dir_entry *proc_net; };
extern struct net init_net;
int  dev_alloc_name(struct net_device *dev, const char *name);
static inline struct net_device *netdev_notifier_info_to_dev(void *info) { return *(struct net_device **)info; }
static inline int register_inetaddr_notifier(void *nb) { (void)nb; return 0; }
static inline int unregister_inetaddr_notifier(void *nb) { (void)nb; return 0; }

struct net_device *alloc_netdev_mqs(int sizeof_priv, const char *name, unsigned char name_assign,
                                    void (*setup)(struct net_device *), unsigned int txqs, unsigned int rxqs);
#define alloc_netdev(sz, name, assign, setup) alloc_netdev_mqs(sz, name, assign, setup, 1, 1)
#define alloc_etherdev(sz) alloc_netdev(sz, "eth%d", 0, NULL)
#define alloc_etherdev_mq(sz, q) alloc_etherdev(sz)
#define alloc_etherdev_mqs(sz, tq, rq) alloc_etherdev(sz)
void free_netdev(struct net_device *dev);
int  register_netdev(struct net_device *dev);
void unregister_netdev(struct net_device *dev);
static inline int  register_netdevice(struct net_device *dev) { return register_netdev(dev); }
static inline void unregister_netdevice(struct net_device *dev) { unregister_netdev(dev); }
static inline void *netdev_priv(const struct net_device *dev) { return dev->priv; }
#define to_net_dev(d) container_of(d, struct net_device, dev)
#define SET_NETDEV_DEV(ndev, pdev) do { (ndev)->dev.parent = (pdev); } while (0)
#define SET_NETDEV_DEVTYPE(ndev, t) do {} while (0)

/* notifier (a driver hálózati esemény-figyelőkre használja) */
struct notifier_block {
	int (*notifier_call)(struct notifier_block *, unsigned long, void *);
	struct notifier_block *next;
	int priority;
};
#define NOTIFY_DONE 0x0000
#define NOTIFY_OK   0x0001
static inline int register_netdevice_notifier(struct notifier_block *nb) { (void)nb; return 0; }
static inline int unregister_netdevice_notifier(struct notifier_block *nb) { (void)nb; return 0; }
struct ethtool_ops;

static inline void netif_start_queue(struct net_device *d) { (void)d; }
static inline void netif_stop_queue(struct net_device *d) { (void)d; }
static inline void netif_wake_queue(struct net_device *d) { (void)d; }
static inline void netif_carrier_on(struct net_device *d) { (void)d; }
static inline void netif_carrier_off(struct net_device *d) { (void)d; }
int netif_rx(struct sk_buff *skb);

/* multi-queue helperek (a driver a netdevice-en át használja) */
struct netdev_queue { struct net_device *dev; };
static inline struct netdev_queue *netdev_get_tx_queue(struct net_device *d, unsigned i) { (void)i; (void)d; return (struct netdev_queue *)0; }
static inline int  netif_tx_queue_stopped(const struct netdev_queue *q) { (void)q; return 0; }
static inline void netif_tx_wake_all_queues(struct net_device *d) { (void)d; }
static inline void netif_tx_start_all_queues(struct net_device *d) { (void)d; }
static inline void netif_tx_stop_all_queues(struct net_device *d) { (void)d; }
static inline void netif_device_attach(struct net_device *d) { (void)d; }
static inline void netif_device_detach(struct net_device *d) { (void)d; }
static inline int  netif_running(const struct net_device *d) { (void)d; return 1; }
static inline int  netif_queue_stopped(const struct net_device *d) { (void)d; return 0; }
static inline int  __netif_subqueue_stopped(const struct net_device *d, u16 q) { (void)d;(void)q; return 0; }
static inline void netif_wake_subqueue(struct net_device *d, u16 q) { (void)d;(void)q; }
static inline void netif_stop_subqueue(struct net_device *d, u16 q) { (void)d;(void)q; }
static inline void netif_start_subqueue(struct net_device *d, u16 q) { (void)d;(void)q; }

/* ===== usbnet / net-usb kiegeszitesek ===== */

enum {
	NETIF_MSG_DRV		= 0x0001,
	NETIF_MSG_PROBE		= 0x0002,
	NETIF_MSG_LINK		= 0x0004,
	NETIF_MSG_TIMER		= 0x0008,
	NETIF_MSG_IFDOWN	= 0x0010,
	NETIF_MSG_IFUP		= 0x0020,
	NETIF_MSG_RX_ERR	= 0x0040,
	NETIF_MSG_TX_ERR	= 0x0080,
	NETIF_MSG_TX_QUEUED	= 0x0100,
	NETIF_MSG_INTR		= 0x0200,
	NETIF_MSG_TX_DONE	= 0x0400,
	NETIF_MSG_RX_STATUS	= 0x0800,
	NETIF_MSG_PKTDATA	= 0x1000,
	NETIF_MSG_HW		= 0x2000,
	NETIF_MSG_WOL		= 0x4000,
};

/* netif_msg_<type>(priv) -> a priv->msg_enable bitje; itt csak no-op (0). */
#define netif_msg_drv(p)	0
#define netif_msg_probe(p)	0
#define netif_msg_link(p)	0
#define netif_msg_timer(p)	0
#define netif_msg_ifdown(p)	0
#define netif_msg_ifup(p)	0
#define netif_msg_rx_err(p)	0
#define netif_msg_tx_err(p)	0
#define netif_msg_tx_queued(p)	0
#define netif_msg_intr(p)	0
#define netif_msg_tx_done(p)	0
#define netif_msg_rx_status(p)	0
#define netif_msg_pktdata(p)	0
#define netif_msg_hw(p)		0
#define netif_msg_wol(p)	0

static inline u32 netif_msg_init(int debug_value, int default_msg_enable_bits)
{ (void)debug_value; return default_msg_enable_bits; }

/* debug makrok — a 'type' token-t (ifdown, rx_err, ...) lenyelik anelkul,
 * hogy kifejtenek/kiertekelnek (nem letezo valtozok). */
#define netif_dbg(priv, type, dev, fmt, ...)   do { } while (0)
#define netif_info(priv, type, dev, fmt, ...)  do { } while (0)
#define netif_warn(priv, type, dev, fmt, ...)  do { } while (0)
#define netif_err(priv, type, dev, fmt, ...)   do { } while (0)
#define netif_notice(priv, type, dev, fmt, ...) do { } while (0)
#define netif_printk(priv, type, level, dev, fmt, ...) do { } while (0)
#define netif_cond_dbg(priv, type, dev, cond, fmt, ...) do { } while (0)

#define netdev_dbg(dev, fmt, ...)    do {} while (0)
#define netdev_info(dev, fmt, ...)   do {} while (0)
#define netdev_warn(dev, fmt, ...)   do {} while (0)
#define netdev_err(dev, fmt, ...)    do {} while (0)
#define netdev_notice(dev, fmt, ...) do {} while (0)
#define netdev_WARN(dev, fmt, ...)   do {} while (0)
#define netdev_warn_once(dev, fmt, ...) do {} while (0)
#define netdev_features_changed(dev) do {} while (0)
#define netif_msg(priv, type, dev, fmt, ...) do {} while (0)

#define netdev_name(dev)	((dev)->name)

static inline void netif_carrier_ok_set(struct net_device *d) { (void)d; }
static inline int  netif_carrier_ok(const struct net_device *d) { (void)d; return 1; }
static inline void netif_tx_lock_bh(struct net_device *d) { (void)d; }
static inline void netif_tx_unlock_bh(struct net_device *d) { (void)d; }
static inline void netif_trans_update(struct net_device *d) { (void)d; }
static inline void netif_napi_add(struct net_device *d, struct napi_struct *n,
				  int (*poll)(struct napi_struct *, int)) { (void)d;(void)n;(void)poll; }
static inline void netif_napi_del(struct napi_struct *n) { (void)n; }
static inline void napi_schedule_irqoff(struct napi_struct *n) { (void)n; }
static inline int  napi_schedule_prep(struct napi_struct *n) { (void)n; return 1; }
static inline void __napi_schedule(struct napi_struct *n) { (void)n; }

void netif_tx_wake_queue(struct netdev_queue *q);

static inline void netdev_update_features(struct net_device *d) { (void)d; }
static inline void netif_set_tso_max_size(struct net_device *d, unsigned int s) { (void)d;(void)s; }

int  dev_change_flags(struct net_device *dev, unsigned int flags, void *ack);
int  netdev_alloc_pcpu_stats_x(struct net_device *dev);
#define netdev_alloc_pcpu_stats(type) (calloc(1, sizeof(type)))
#define dev_get_tstats64 NULL

void dev_kfree_skb(struct sk_buff *skb);
struct net_device *dev_get_drvdata_net(struct device *dev);

#define SINGLE_DEPTH_NESTING 1

/* multicast / unicast cim-listak (a driverek set_rx_mode-ja jarja be) */
struct netdev_hw_addr {
	struct list_head list;
	unsigned char addr[6];
	unsigned char type;
};
struct netdev_hw_addr_list {
	struct list_head list;
	int count;
};

static inline int netdev_mc_count(const struct net_device *dev) { (void)dev; return 0; }
static inline int netdev_mc_empty(const struct net_device *dev) { (void)dev; return 1; }
static inline int netdev_uc_count(const struct net_device *dev) { (void)dev; return 0; }
static inline int netdev_uc_empty(const struct net_device *dev) { (void)dev; return 1; }

#define netdev_for_each_mc_addr(ha, dev) \
	for (ha = NULL; ha != NULL; )
#define netdev_for_each_uc_addr(ha, dev) \
	for (ha = NULL; ha != NULL; )
#define netdev_hw_addr_list_for_each(ha, l) \
	for (ha = NULL; ha != NULL; )
#define netdev_for_each_upper_dev_rcu(dev, updev, iter) \
	for (updev = NULL, (void)(iter); updev != NULL; )
#define netdev_for_each_lower_dev(dev, ldev, iter) \
	for (ldev = NULL, (void)(iter); ldev != NULL; )

int  netdev_upper_dev_link(struct net_device *dev, struct net_device *upper, void *extack);
void netdev_upper_dev_unlink(struct net_device *dev, struct net_device *upper);
struct net_device *netdev_master_upper_dev_get_rcu(struct net_device *dev);

static inline void dev_uc_unsync(struct net_device *to, struct net_device *from) { (void)to;(void)from; }
static inline void dev_mc_unsync(struct net_device *to, struct net_device *from) { (void)to;(void)from; }

#define netdev_printk(level, dev, fmt, ...) do {} while (0)
#define netif_msg_enable(p) 0
#define netif_carrier_event(dev) do {} while (0)

void dev_close(struct net_device *dev);
int  dev_open(struct net_device *dev, void *extack);

#endif

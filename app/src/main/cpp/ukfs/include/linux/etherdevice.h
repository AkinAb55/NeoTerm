#ifndef _UK_LINUX_ETHERDEVICE_H
#define _UK_LINUX_ETHERDEVICE_H
#include <linux/netdevice.h>
/* if_ether NEM jon ide a rtl8812au-buildben (sajat byteorder/generic.h-ja a mi
 * konstans htons-unkat undef-eli, ezert a case-label-ek elromolnanak). A net-usb
 * build (nincs UKERNEL_DRIVER_BUILD) megkapja az ETH_P es ethhdr definiciokat. */
#ifndef UKERNEL_DRIVER_BUILD
#include <linux/if_ether.h>
#endif
#include <string.h>

#ifndef ETH_ALEN
#define ETH_ALEN	6
#endif
#ifndef ETH_HLEN
#define ETH_HLEN	14
#endif
#ifndef ETH_FCS_LEN
#define ETH_FCS_LEN	4
#endif
#ifndef ETH_DATA_LEN
#define ETH_DATA_LEN	1500
#endif
#ifndef ETH_FRAME_LEN
#define ETH_FRAME_LEN	1514
#endif
#ifndef ETH_ZLEN
#define ETH_ZLEN	60
#endif
#ifndef VLAN_ETH_HLEN
#define VLAN_ETH_HLEN	18
#endif

static inline int is_zero_ether_addr(const u8 *a){ return !(a[0]|a[1]|a[2]|a[3]|a[4]|a[5]); }
static inline int is_multicast_ether_addr(const u8 *a){ return a[0]&1; }
static inline int is_broadcast_ether_addr(const u8 *a){ return (a[0]&a[1]&a[2]&a[3]&a[4]&a[5])==0xff; }
static inline void eth_broadcast_addr(u8 *a){ memset(a,0xff,6); }
static inline void eth_zero_addr(u8 *a){ memset(a,0,6); }
static inline void ether_addr_copy(u8 *d,const u8 *s){ memcpy(d,s,6); }
static inline int ether_addr_equal(const u8 *a,const u8 *b){ return memcmp(a,b,6)==0; }
static inline int is_valid_ether_addr(const u8 *a){ return !is_multicast_ether_addr(a) && !is_zero_ether_addr(a); }
static inline void random_ether_addr(u8 *a){ memset(a,0,6); a[0]=0x02; }
static inline void eth_hw_addr_random(struct net_device *d){ d->dev_addr[0]=0x02; }
static inline void eth_hw_addr_set(struct net_device *d, const u8 *a){ memcpy(d->dev_addr,a,6); }
static inline __be16 eth_type_trans(struct sk_buff *skb, struct net_device *dev){ (void)dev; skb_pull(skb,14); return 0; }
static inline struct ethhdr *eth_hdr(const struct sk_buff *skb){ return (struct ethhdr *)skb->data; }
static inline struct ethhdr *skb_eth_hdr(const struct sk_buff *skb){ return (struct ethhdr *)skb->data; }
static inline __be16 eth_get_headlen(const struct net_device *d, void *data, unsigned int len){ (void)d;(void)data;(void)len; return 14; }
static inline int eth_validate_addr(struct net_device *dev){ return is_valid_ether_addr(dev->dev_addr) ? 0 : -22; }
static inline int eth_mac_addr(struct net_device *dev, void *p)
{ struct sockaddr *sa = p; memcpy(dev->dev_addr, sa->sa_data, 6); return 0; }
static inline int eth_change_mtu(struct net_device *dev, int new_mtu){ dev->mtu = new_mtu; return 0; }
static inline void eth_random_addr(u8 *a){ a[0]=0x02; a[1]=a[2]=a[3]=a[4]=a[5]=0; }
static inline int ether_addr_equal_64bits(const u8 *a, const u8 *b){ return memcmp(a,b,6)==0; }
static inline int is_local_ether_addr(const u8 *a){ return a[0]&0x02; }
static inline int is_unicast_ether_addr(const u8 *a){ return !is_multicast_ether_addr(a); }
static inline void eth_hw_addr_inherit(struct net_device *d, struct net_device *s){ memcpy(d->dev_addr, s->dev_addr, 6); }
void eth_hw_addr_crc(void *ha);
static inline u32 ether_crc(int length, unsigned char *data){ (void)length;(void)data; return 0; }
static inline u32 ether_crc_le(int length, unsigned char *data){ (void)length;(void)data; return 0; }
void ether_setup(struct net_device *dev);
struct net_device *alloc_etherdev_real(int sizeof_priv);
#endif

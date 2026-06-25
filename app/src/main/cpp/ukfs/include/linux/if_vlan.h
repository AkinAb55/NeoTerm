#ifndef _UK_LINUX_IF_VLAN_H
#define _UK_LINUX_IF_VLAN_H
#include <linux/types.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#define VLAN_HLEN		4
#define VLAN_ETH_HLEN		18
#define VLAN_ETH_FRAME_LEN	1518
#define VLAN_N_VID		4096
#define VLAN_PRIO_MASK		0xe000
#define VLAN_PRIO_SHIFT		13
#define VLAN_CFI_MASK		0x1000
#define VLAN_VID_MASK		0x0fff
struct vlan_ethhdr {
	unsigned char h_dest[6];
	unsigned char h_source[6];
	__be16 h_vlan_proto;
	__be16 h_vlan_TCI;
	__be16 h_vlan_encapsulated_proto;
};
struct vlan_hdr { __be16 h_vlan_TCI; __be16 h_vlan_encapsulated_proto; };
static inline int vlan_get_tag(const struct sk_buff *skb, u16 *vlan_tci) { (void)skb; *vlan_tci = 0; return -1; }
static inline struct sk_buff *vlan_hwaccel_push_inside(struct sk_buff *skb) { return skb; }
static inline bool eth_type_vlan(__be16 ethertype) { return ethertype == 0x8100 || ethertype == 0x88a8; }
static inline struct vlan_ethhdr *vlan_eth_hdr(const struct sk_buff *skb) { return (struct vlan_ethhdr *)skb->data; }
#endif

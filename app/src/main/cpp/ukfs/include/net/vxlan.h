#ifndef _UK_NET_VXLAN_H
#define _UK_NET_VXLAN_H
#include <linux/types.h>
struct net_device;
static inline bool netif_is_vxlan(const struct net_device *dev) { (void)dev; return false; }
#endif

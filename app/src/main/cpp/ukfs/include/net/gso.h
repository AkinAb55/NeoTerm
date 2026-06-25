#ifndef _UK_NET_GSO_H
#define _UK_NET_GSO_H
#include <linux/skbuff.h>
static inline struct sk_buff *skb_mac_gso_segment(struct sk_buff *skb, netdev_features_t f) { (void)f; return skb; }
#endif

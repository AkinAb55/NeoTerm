#ifndef _UK_NET_IP_H
#define _UK_NET_IP_H
#include <linux/types.h>
#include <linux/ip.h>
static inline unsigned int ip_hdrlen(const struct sk_buff *skb) { (void)skb; return 20; }
#endif

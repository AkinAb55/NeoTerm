#ifndef _UK_NET_UDP_H
#define _UK_NET_UDP_H
#include <linux/types.h>
struct udphdr { __be16 source; __be16 dest; __be16 len; __sum16 check; };
static inline struct udphdr *udp_hdr(const struct sk_buff *skb) { return (struct udphdr *)(skb)->data; }
#endif

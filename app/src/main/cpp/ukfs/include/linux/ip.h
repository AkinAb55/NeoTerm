#ifndef _UK_LINUX_IP_H
#define _UK_LINUX_IP_H
#include <linux/types.h>
#ifndef IPPROTO_TCP
#define IPPROTO_IP   0
#define IPPROTO_ICMP 1
#define IPPROTO_IGMP 2
#define IPPROTO_TCP  6
#define IPPROTO_UDP  17
#define IPPROTO_IPV6 41
#define IPPROTO_GRE  47
#define IPPROTO_RAW  255
#endif
struct iphdr { u8 ihl:4, version:4; u8 tos; __be16 tot_len; __be16 id; __be16 frag_off; u8 ttl; u8 protocol; __be16 check; __be32 saddr, daddr; };
#include <linux/skbuff.h>
static inline struct iphdr *ip_hdr(const struct sk_buff *skb){ return (struct iphdr *)skb->data; }
static inline unsigned int ip_transport_len(const struct sk_buff *skb){ (void)skb; return 0; }
#endif

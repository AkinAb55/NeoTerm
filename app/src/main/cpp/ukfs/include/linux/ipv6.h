#ifndef _UK_LINUX_IPV6_H
#define _UK_LINUX_IPV6_H
#include <linux/types.h>
struct in6_addr { union { __u8 u6_addr8[16]; __be16 u6_addr16[8]; __be32 u6_addr32[4]; } in6_u; };
struct ipv6hdr {
	__u8 priority:4, version:4;
	__u8 flow_lbl[3];
	__be16 payload_len;
	__u8 nexthdr;
	__u8 hop_limit;
	struct in6_addr saddr;
	struct in6_addr daddr;
};
static inline struct ipv6hdr *ipv6_hdr(const struct sk_buff *skb) { return (struct ipv6hdr *)(skb)->data; }
#endif

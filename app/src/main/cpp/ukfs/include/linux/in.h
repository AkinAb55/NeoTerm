#ifndef _UK_LINUX_IN_H
#define _UK_LINUX_IN_H
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
struct in_addr { __be32 s_addr; };
#endif

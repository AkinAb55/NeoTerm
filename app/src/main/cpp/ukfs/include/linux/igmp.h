#ifndef _UK_LINUX_IGMP_H
#define _UK_LINUX_IGMP_H
#include <linux/types.h>
struct igmphdr { __u8 type; __u8 code; __sum16 csum; __be32 group; };
#define IGMP_HOST_MEMBERSHIP_QUERY  0x11
#define IGMP_HOST_MEMBERSHIP_REPORT 0x12
#endif

#ifndef _UK_NET_IP6_CHECKSUM_H
#define _UK_NET_IP6_CHECKSUM_H
#include <linux/types.h>
static inline __sum16 csum_ipv6_magic(const void *saddr, const void *daddr, __u32 len, __u8 proto, __wsum sum) { (void)saddr;(void)daddr;(void)len;(void)proto; return (__sum16)sum; }
#endif

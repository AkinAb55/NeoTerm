#ifndef _UK_NET_SELFTESTS_H
#define _UK_NET_SELFTESTS_H
#include <linux/types.h>
struct net_device; struct ethtool_test;
static inline void net_selftest(struct net_device *dev, struct ethtool_test *etest, u64 *buf) { (void)dev;(void)etest;(void)buf; }
static inline int net_selftest_get_count(void) { return 0; }
static inline void net_selftest_get_strings(u8 *data) { (void)data; }
#endif

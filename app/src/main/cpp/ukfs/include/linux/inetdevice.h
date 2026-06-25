#ifndef _UK_LINUX_INETDEVICE_H
#define _UK_LINUX_INETDEVICE_H
#include <linux/types.h>
struct net_device;
struct in_device { struct in_ifaddr *ifa_list; struct net_device *dev; };
struct in_ifaddr { struct in_ifaddr *ifa_next; struct in_device *ifa_dev; __be32 ifa_address; __be32 ifa_mask; __be32 ifa_broadcast; };
#define in_dev_for_each_ifa_rcu(ifa, in_dev) for (ifa = (in_dev)->ifa_list; ifa; ifa = ifa->ifa_next)
#define in_dev_for_each_ifa_rtnl(ifa, in_dev) for (ifa = (in_dev)->ifa_list; ifa; ifa = ifa->ifa_next)
#endif

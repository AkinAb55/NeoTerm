#ifndef _UK_LINUX_IF_H
#define _UK_LINUX_IF_H
#include <linux/types.h>
#include <linux/socket.h>
#include <linux/sockios.h>
#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif
#define IFHWADDRLEN 6
struct ifmap { unsigned long mem_start, mem_end; unsigned short base_addr; unsigned char irq, dma, port; };
struct ifreq {
	char ifr_name[IFNAMSIZ];
	union {
		struct sockaddr ifr_addr;
		struct sockaddr ifr_hwaddr;
		short  ifr_flags;
		int    ifr_ifindex;
		int    ifr_mtu;
		char  *ifr_data;
		char   ifr_slave[IFNAMSIZ];
		char   ifr_ifru[28];   /* generic helyfoglalo (if_mii) */
	} ifr_ifru_union;
};
/* a kernel az 'ifr_*' tag-eket az unio-mezokre kepezi */
#define ifr_addr	ifr_ifru_union.ifr_addr
#define ifr_hwaddr	ifr_ifru_union.ifr_hwaddr
#define ifr_flags	ifr_ifru_union.ifr_flags
#define ifr_ifindex	ifr_ifru_union.ifr_ifindex
#define ifr_mtu		ifr_ifru_union.ifr_mtu
#define ifr_data	ifr_ifru_union.ifr_data
#define ifr_slave	ifr_ifru_union.ifr_slave
#define ifr_ifru	ifr_ifru_union.ifr_ifru
#endif

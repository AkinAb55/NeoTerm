#ifndef _UK_NET_NDISC_H
#define _UK_NET_NDISC_H
#include <linux/types.h>
#define NDISC_NEIGHBOUR_SOLICITATION 135
#define NDISC_NEIGHBOUR_ADVERTISEMENT 136
struct nd_msg { __u8 icmph[8]; struct { __u8 x[16]; } target; };
#endif

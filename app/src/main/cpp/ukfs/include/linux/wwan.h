#ifndef _UK_LINUX_WWAN_H
#define _UK_LINUX_WWAN_H
#include <linux/types.h>
enum wwan_port_type {
	WWAN_PORT_AT, WWAN_PORT_MBIM, WWAN_PORT_QMI, WWAN_PORT_QCDM,
	WWAN_PORT_FIREHOSE, WWAN_PORT_XMMRPC, WWAN_PORT_UNKNOWN, WWAN_PORT_MAX = WWAN_PORT_UNKNOWN,
};
struct wwan_port;
#endif

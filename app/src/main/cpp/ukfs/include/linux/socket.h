#ifndef _UK_LINUX_SOCKET_H
#define _UK_LINUX_SOCKET_H
#include <linux/types.h>
typedef unsigned short __kernel_sa_family_t;
typedef unsigned short sa_family_t;
struct sockaddr { unsigned short sa_family; char sa_data[14]; };
struct sockaddr_storage { unsigned short ss_family; char __data[126]; };
#endif

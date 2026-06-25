#ifndef _UK_LINUX_BUG_H
#define _UK_LINUX_BUG_H
#include <linux/kernel.h>
#ifndef BUILD_BUG
#define BUILD_BUG() do{}while(0)
#endif
#define BUILD_BUG_ON_INVALID(e) ((void)(sizeof((long)(e))))
#endif

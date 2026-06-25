/* uKernel hamis <linux/namei.h> — név-feloldás stub. */
#ifndef _UK_LINUX_NAMEI_H
#define _UK_LINUX_NAMEI_H
#include <linux/fs.h>

#define LOOKUP_FOLLOW		0x0001
#define LOOKUP_DIRECTORY	0x0002
#define LOOKUP_RCU		0x0004
#define LOOKUP_CREATE		0x0200
#define LOOKUP_RENAME_TARGET	0x0800

enum { LAST_NORM, LAST_ROOT, LAST_DOT, LAST_DOTDOT };
#endif

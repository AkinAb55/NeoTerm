#ifndef _UK_LINUX_CAPABILITY_H
#define _UK_LINUX_CAPABILITY_H
#include <linux/types.h>
#define CAP_CHOWN 0
#define CAP_DAC_OVERRIDE 1
#define CAP_DAC_READ_SEARCH 2
#define CAP_FOWNER 3
#define CAP_FSETID 4
#define CAP_LINUX_IMMUTABLE 9
#define CAP_SYS_RESOURCE 24
#define CAP_FSETID 4
#define CAP_MKNOD 27
#define CAP_NET_ADMIN 12
#define CAP_SYS_ADMIN 21
#define CAP_NET_RAW   13
/* a 'capable()' a uaccess.h-ban van (int visszateres) — itt nem definialjuk ujra */
static inline bool ns_capable(void *ns, int cap) { (void)ns;(void)cap; return true; }
#endif

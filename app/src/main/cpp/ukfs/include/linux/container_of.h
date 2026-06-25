#ifndef _UK_LINUX_CONTAINER_OF_H
#define _UK_LINUX_CONTAINER_OF_H
#include <linux/kernel.h>   /* container_of, offsetof */
#ifndef container_of
#define container_of(ptr, type, member) ((type *)((char *)(ptr) - offsetof(type, member)))
#endif
#define container_of_const(ptr, type, member) container_of(ptr, type, member)
#endif

#ifndef _UK_LINUX_POISON_H
#define _UK_LINUX_POISON_H
#define LIST_POISON1 ((void *)0x100)
#define LIST_POISON2 ((void *)0x122)
#define POISON_INUSE 0x5a
#define POISON_FREE 0x6b
#define POISON_END 0xa5
#endif

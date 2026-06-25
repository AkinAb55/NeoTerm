#ifndef _UK_LINUX_RANDOM_H
#define _UK_LINUX_RANDOM_H
#include <linux/types.h>
u32 get_random_u32(void);
void get_random_bytes(void *buf, int nbytes);
static inline u32 prandom_u32(void){ return get_random_u32(); }
#endif

#ifndef _UK_LINUX_UUID_H
#define _UK_LINUX_UUID_H
#include <linux/types.h>
typedef struct { u8 b[16]; } uuid_t;
typedef struct { u8 b[16]; } guid_t;
#define UUID_SIZE 16
static inline void uuid_copy(uuid_t *d, const uuid_t *s) { *d = *s; }
static inline int uuid_equal(const uuid_t *a, const uuid_t *b) { int i; for(i=0;i<16;i++) if(a->b[i]!=b->b[i]) return 0; return 1; }
static inline void generate_random_uuid(unsigned char uuid[16]) { int i; for(i=0;i<16;i++) uuid[i]=i; }
#endif

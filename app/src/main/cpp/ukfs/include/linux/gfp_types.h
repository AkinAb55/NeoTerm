#ifndef _UK_LINUX_GFP_TYPES_H
#define _UK_LINUX_GFP_TYPES_H
#include <linux/types.h>
#ifndef ___GFP_FS
#define ___GFP_FS 0x80u
#endif
#ifndef __GFP_FS
#define __GFP_FS 0x80u
#endif
#ifndef __GFP_MOVABLE
#define __GFP_MOVABLE 0x8000u
#endif
#ifndef __GFP_IO
#define __GFP_IO 0x40u
#endif
#define __GFP_NORETRY 0x10000u
#define __GFP_NOMEMALLOC 0x20000u
#define __GFP_HARDWALL 0x40000u
#define GFP_NOWAIT_UK 0x800u
#endif

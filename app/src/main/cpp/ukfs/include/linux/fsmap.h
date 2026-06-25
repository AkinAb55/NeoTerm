#ifndef _UK_LINUX_FSMAP_H
#define _UK_LINUX_FSMAP_H
#include <linux/types.h>
struct fsmap { __u32 fmr_device; __u32 fmr_flags; __u64 fmr_physical; __u64 fmr_owner; __u64 fmr_offset; __u64 fmr_length; __u64 fmr_reserved[3]; };
struct fsmap_head { __u32 fmh_iflags; __u32 fmh_oflags; __u32 fmh_count; __u32 fmh_entries; __u64 fmh_reserved[6]; struct fsmap fmh_keys[2]; struct fsmap fmh_recs[]; };
#define FS_IOC_GETFSMAP _IOWR('X', 59, struct fsmap_head)
#endif

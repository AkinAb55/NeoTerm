/* uKernel hamis <linux/fileattr.h>. */
#ifndef _UK_LINUX_FILEATTR_H
#define _UK_LINUX_FILEATTR_H
#include <linux/types.h>
struct file_kattr { u32 flags; u32 fsx_xflags; u32 fsx_extsize; u32 fsx_nextents; u32 fsx_projid; u32 fsx_cowextsize; bool flags_valid:1; bool fsx_valid:1; };
struct fileattr { u32 flags; u32 fsx_xflags; u32 fsx_extsize; u32 fsx_nextents; u32 fsx_projid; u32 fsx_cowextsize; bool flags_valid:1; bool fsx_valid:1; };
#define FS_IMMUTABLE_FL 0x00000010
#define FS_APPEND_FL    0x00000020
#define FS_NODUMP_FL    0x00000040
#define FS_SYSTEM_FL    0x00000800
#define FS_ARCHIVE_FL   0x00000800
#define FS_HIDDEN_FL    0x00010000
#define FS_COMPR_FL     0x00000004
static inline void fileattr_fill_flags(struct fileattr *fa, u32 flags) { fa->flags = flags; fa->flags_valid = true; }
static inline void fileattr_fill_xflags(struct fileattr *fa, u32 xflags) { fa->fsx_xflags = xflags; fa->fsx_valid = true; }
#endif

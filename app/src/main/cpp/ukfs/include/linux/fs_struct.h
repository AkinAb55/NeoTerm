/* uKernel hamis <linux/fs_struct.h> — fs_struct stub. */
#ifndef _UK_LINUX_FS_STRUCT_H
#define _UK_LINUX_FS_STRUCT_H
#include <linux/fs.h>   /* struct path */
struct fs_struct { int users; struct path root; struct path pwd; int umask; };
#endif

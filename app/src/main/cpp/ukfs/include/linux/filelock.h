/* uKernel hamis <linux/filelock.h> — fájl-zárolás stub. */
#ifndef _UK_LINUX_FILELOCK_H
#define _UK_LINUX_FILELOCK_H
#include <linux/types.h>
struct file_lock { int fl_type; loff_t fl_start; loff_t fl_end; };
#define F_RDLCK 0
#define F_WRLCK 1
#define F_UNLCK 2
static inline int locks_mandatory_area(void *i, void *f, loff_t s, loff_t e, unsigned char t) { return 0; }
#endif

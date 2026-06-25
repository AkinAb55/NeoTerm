/* uKernel hamis <linux/uaccess.h>.
 * Userspace-ben a "user pointer" valódi pointer -> copy = memcpy. A proxy a
 * kliens pufferét egy lokális pufferbe másolja, és annak címét adja át a
 * drivernek arg-ként; így a copy_from/to_user helyesen működik. */
#ifndef _UK_LINUX_UACCESS_H
#define _UK_LINUX_UACCESS_H

#include <linux/types.h>
#include <linux/compiler.h>   /* __user */
#include <linux/capability.h>
#include <string.h>

static inline unsigned long copy_to_user(void __user *to, const void *from, unsigned long n)
{ memcpy((void *)to, from, n); return 0; }
static inline unsigned long copy_from_user(void *to, const void __user *from, unsigned long n)
{ memcpy(to, (const void *)from, n); return 0; }

#define get_user(x, ptr)  ({ (x) = *(ptr); 0; })
#define put_user(x, ptr)  ({ *(ptr) = (x); 0; })

static inline long access_ok(const void __user *p, unsigned long n) { (void)p; (void)n; return 1; }

/* capability — userspace shim: mindig engedélyezett (usb-serial serial_set_serial) */
#define CAP_SYS_ADMIN 21
static inline int capable(int cap) { (void)cap; return 1; }

#endif

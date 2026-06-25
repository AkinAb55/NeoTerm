#ifndef _UK_LINUX_KSTRTOX_H
#define _UK_LINUX_KSTRTOX_H
#include <linux/types.h>
int kstrtoul(const char *s, unsigned int base, unsigned long *res);
int kstrtol(const char *s, unsigned int base, long *res);
int kstrtouint(const char *s, unsigned int base, unsigned int *res);
int kstrtoint(const char *s, unsigned int base, int *res);
int kstrtou8(const char *s, unsigned int base, u8 *res);
int kstrtou16(const char *s, unsigned int base, u16 *res);
int kstrtbool_dummy(void);
int kstrtobool(const char *s, bool *res);
#endif

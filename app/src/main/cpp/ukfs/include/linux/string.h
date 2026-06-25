/* uKernel hamis <linux/string.h> — a libc string.h-ra építve + kernel-extrák. */
#ifndef _UK_LINUX_STRING_H
#define _UK_LINUX_STRING_H

#include <string.h>     /* host: memcpy, memset, strlen, strcmp, ... */
#include <linux/types.h>

/* kernel-specifikus kiegészítések */
size_t strlcpy(char *dst, const char *src, size_t size);
size_t strlcat(char *dst, const char *src, size_t size);
char  *kstrdup(const char *s, gfp_t gfp);
char  *strim(char *s);
static inline char *skip_spaces(const char *str)
{ while (*str == ' ' || *str == '\t' || *str == '\n') ++str; return (char *)str; }

/* strscpy — biztonságos, mindig null-terminált másolás; visszaadja a másolt
 * hosszt, vagy -E2BIG ha csonkolt (a driverek legtöbbször csak >=0-t néznek).
 * A kernelben strscpy(dst, src[, size]) — ha a size hiányzik, sizeof(dst). */
static inline ssize_t __strscpy(char *dst, const char *src, size_t size)
{
	size_t len;
	if (!size)
		return -7 /* -E2BIG */;
	len = strlen(src);
	if (len >= size) {
		memcpy(dst, src, size - 1);
		dst[size - 1] = '\0';
		return -7 /* -E2BIG */;
	}
	memcpy(dst, src, len + 1);
	return (ssize_t)len;
}
#define __strscpy3(dst, src, size) __strscpy((dst), (src), (size))
#define __strscpy2(dst, src)       __strscpy((dst), (src), sizeof(dst))
#define __strscpy_pick(_1, _2, _3, NAME, ...) NAME
#ifndef strscpy
#define strscpy(...) __strscpy_pick(__VA_ARGS__, __strscpy3, __strscpy2)(__VA_ARGS__)
#endif

#endif

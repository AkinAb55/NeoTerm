/* uKernel — kernel-specifikus string-kiegészítések. */
#include <linux/string.h>
#include <linux/slab.h>
#include <string.h>
#include <ctype.h>

size_t strlcpy(char *dst, const char *src, size_t size)
{
	size_t srclen = strlen(src);
	if (size) {
		size_t n = srclen < size - 1 ? srclen : size - 1;
		memcpy(dst, src, n);
		dst[n] = '\0';
	}
	return srclen;
}

size_t strlcat(char *dst, const char *src, size_t size)
{
	size_t dlen = strnlen(dst, size);
	if (dlen == size) return size + strlen(src);
	return dlen + strlcpy(dst + dlen, src, size - dlen);
}

char *kstrdup(const char *s, gfp_t gfp)
{
	if (!s) return NULL;
	size_t n = strlen(s) + 1;
	char *p = kmalloc(n, gfp);
	if (p) memcpy(p, s, n);
	return p;
}

char *strim(char *s)
{
	char *e;
	while (isspace((unsigned char)*s)) s++;
	if (!*s) return s;
	e = s + strlen(s) - 1;
	while (e > s && isspace((unsigned char)*e)) *e-- = '\0';
	return s;
}

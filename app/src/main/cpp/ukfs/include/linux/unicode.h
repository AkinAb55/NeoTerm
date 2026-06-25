#ifndef _UK_LINUX_UNICODE_H
#define _UK_LINUX_UNICODE_H
#include <linux/types.h>
struct unicode_map;
struct qstr;
int utf8_strncmp(const struct unicode_map *um, const struct qstr *s1, const struct qstr *s2);
int utf8_strncasecmp(const struct unicode_map *um, const struct qstr *s1, const struct qstr *s2);
int utf8_casefold(const struct unicode_map *um, const struct qstr *str, unsigned char *dest, size_t dlen);
struct unicode_map *utf8_load(unsigned int version);
void utf8_unload(struct unicode_map *um);
#endif

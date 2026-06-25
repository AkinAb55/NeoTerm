#ifndef _UK_LINUX_PARSER_H
#define _UK_LINUX_PARSER_H
struct match_token { int token; const char *pattern; };
typedef struct { char *from; char *to; } substring_t;
int match_token(char *s, const struct match_token table[], substring_t args[]);
int match_int(substring_t *s, int *result);
int match_u64(substring_t *s, u64 *result);
int match_hex(substring_t *s, int *result);
int match_strdup(const substring_t *s, char **dst);
size_t match_strlcpy(char *dest, const substring_t *src, size_t size);
#endif

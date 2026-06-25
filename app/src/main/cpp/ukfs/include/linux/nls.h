/* uKernel hamis <linux/nls.h> — kódlap/charset-konverzió felülete. */
#ifndef _UK_LINUX_NLS_H
#define _UK_LINUX_NLS_H
#include <linux/types.h>

/* a wchar_t a kernelben 16 bites Unicode-kódpont */
#ifndef __KERNEL_WCHAR
typedef u16 wchar_t_nls;
#endif

struct nls_table {
	const char *charset;
	const char *alias;
	int (*uni2char)(wchar_t uni, unsigned char *out, int boundlen);
	int (*char2uni)(const unsigned char *rawstring, int boundlen, wchar_t *uni);
	const unsigned char *charset2lower;
	const unsigned char *charset2upper;
	struct module *owner;
	struct nls_table *next;
};

enum utf16_endian { UTF16_HOST_ENDIAN, UTF16_LITTLE_ENDIAN, UTF16_BIG_ENDIAN };
#define NLS_MAX_CHARSET_SIZE 6

extern struct nls_table *load_nls(const char *charset);
extern void unload_nls(struct nls_table *);
extern struct nls_table *load_nls_default(void);

extern int utf8s_to_utf16s(const u8 *s, int len, enum utf16_endian endian, wchar_t *pwcs, int maxlen);
extern int utf16s_to_utf8s(const wchar_t *pwcs, int len, enum utf16_endian endian, u8 *s, int maxlen);

static inline unsigned char nls_tolower(struct nls_table *t, unsigned char c)
{ return t->charset2lower && t->charset2lower[c] ? t->charset2lower[c] : c; }
static inline unsigned char nls_toupper(struct nls_table *t, unsigned char c)
{ return t->charset2upper && t->charset2upper[c] ? t->charset2upper[c] : c; }
static inline int nls_strnicmp(struct nls_table *t, const unsigned char *s1,
			       const unsigned char *s2, int len)
{ while (len--) { if (nls_tolower(t, *s1++) != nls_tolower(t, *s2++)) return 1; } return 0; }
#endif

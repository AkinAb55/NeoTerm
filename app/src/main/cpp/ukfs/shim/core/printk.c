/* uKernel — printk és alap formázók/parszolók. */
#include <linux/kernel.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

static int g_loglevel = -1;  /* -1 = még nincs init; első használatkor a UK_LOGLEVEL env / 7 */

void ukernel_set_loglevel(int level) { g_loglevel = level; }

/* ===== "dmesg" — a kernel-log-puffer fájlban (UK_DMESG), hogy a uksh `dmesg`-je
 * MINDEN uk-processz (backend + preload-ok) printk-jét egy helyen lássa. A fájlba a
 * console-loglevel-től FÜGGETLENÜL írunk (mint a valódi log_buf); csak a stderr szűrt.
 * Egyetlen write()-tal (O_APPEND), hogy a párhuzamos processzek sorai ne keveredjenek. */
/* a kernel-log fix alapértelmezett útja, hogy MINDEN uk-processz (ukd, userver, preload-ok)
 * env NÉLKÜL is ide írjon — a uksh `dmesg`-je ezt olvassa. UK_DMESG felülírja; UK_DMESG=off kikapcsol. */
#define UK_DMESG_DEFAULT "/tmp/uk_dmesg.log"
#define UK_DMESG_CAP     (4 * 1024 * 1024)   /* méret-cap: e fölött elölről (ring-szerű, korlátos) */
static const char *dmesg_path(void)
{
	const char *p = getenv("UK_DMESG");
	if (p && !strcmp(p, "off")) return NULL;
	return (p && *p) ? p : UK_DMESG_DEFAULT;
}
static unsigned long g_dm_bytes;   /* e processz által írt bájtok (cap-hez) */
static FILE *dmesg_fp(void)
{
	static FILE *fp = (FILE *)-1;
	if (fp == (FILE *)-1) {
		const char *path = dmesg_path();
		if (!path) { fp = NULL; return fp; }
		struct stat st;   /* induláskor: ha már túl nagy, kezdjük elölről */
		fp = (stat(path, &st) == 0 && st.st_size > UK_DMESG_CAP) ? fopen(path, "w") : fopen(path, "a");
	}
	return fp;
}
static unsigned long g_dm_t0;
static void uk_dmesg_write(int level, const char *s, size_t n)
{
	FILE *fp = dmesg_fp(); if (!fp) return;
	struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
	unsigned long ms = (unsigned long)ts.tv_sec * 1000UL + ts.tv_nsec / 1000000UL;
	if (!g_dm_t0) g_dm_t0 = ms; unsigned long d = ms - g_dm_t0;
	char line[1400];
	int L = snprintf(line, sizeof line, "[%5lu.%03lu] ", d / 1000, d % 1000);
	if (L < 0) return; size_t cp = n; if ((size_t)L + cp > sizeof line - 1) cp = sizeof line - 1 - L;
	memcpy(line + L, s, cp); L += (int)cp;
	if (L > 0 && line[L-1] != '\n' && (size_t)L < sizeof line - 1) line[L++] = '\n';
	/* futásidejű méret-cap: e processz írásai is korlátozva (hosszan futó ukd/userver) */
	g_dm_bytes += (unsigned long)L;
	if (g_dm_bytes > UK_DMESG_CAP) { const char *path = dmesg_path(); if (path) { FILE *nf = freopen(path, "w", fp); if (nf) fp = nf; } g_dm_bytes = (unsigned long)L; }
	fwrite(line, 1, (size_t)L, fp); fflush(fp);
}

/* Lusta init: a UK_LOGLEVEL env-ből (0=csak hiba … 7=minden); alap 7. Így a uKernel-shell
 * UK_LOGLEVEL=3-mal elnyomhatja az INFO driver-spam-et (module_init stb.), a hibákat meghagyva. */
static int loglevel(void)
{
	if (g_loglevel < 0) { const char *e = getenv("UK_LOGLEVEL"); g_loglevel = e ? atoi(e) : 7; }
	return g_loglevel;
}

/* A printk szint a fmt elején KERN_SOH "<n>" formában jön. Kiszedjük és
 * eldobjuk a túl részletes sorokat a szint szerint. */
/* struct va_format: a kernel %pV-hez (beágyazott fmt + args) */
struct uk_va_format { const char *fmt; va_list *va; };

/* Kernel-tudatos formázó: a host vsnprintf nem érti a %pV-t (és más %p-kiterjesztést).
 * Spec-enként haladunk; a %pV-t rekurzívan formázzuk, a többit a host vsnprintf-fel. */
static int uk_vformat(FILE *out, const char *fmt, va_list args)
{
	int total = 0;
	const char *p = fmt;
	char spec[64];
	while (*p) {
		if (*p != '%') { fputc(*p++, out); total++; continue; }
		const char *start = p++;            /* a '%' */
		/* flags, width, precision, length-módosítók átugrása */
		while (*p && strchr("-+ #0", *p)) p++;
		while (*p >= '0' && *p <= '9') p++;
		if (*p == '.') { p++; while (*p >= '0' && *p <= '9') p++; }
		const char *lenmod = p;
		while (*p && strchr("hljztL", *p)) p++;
		char conv = *p;
		/* %pV: beágyazott va_format */
		if (conv == 'p' && p[1] == 'V') {
			struct uk_va_format *vaf = va_arg(args, struct uk_va_format *);
			va_list copy; va_copy(copy, *vaf->va);
			total += uk_vformat(out, vaf->fmt, copy);
			va_end(copy);
			p += 2;
			continue;
		}
		/* egyéb %p-kiterjesztések (pl. %pa, %pK) -> sima pointer */
		if (conv == 'p' && p[1] && strchr("aKxsbBeEfgGhIMmRr", p[1]) ) p++;
		/* a spec kimásolása és egyetlen-argumentumos formázás */
		size_t slen = (size_t)(p - start + 1);
		if (slen >= sizeof(spec)) slen = sizeof(spec) - 1;
		memcpy(spec, start, slen); spec[slen] = 0;
		(void)lenmod;
		char buf[1024];
		int n = 0;
		switch (conv) {
		case 'd': case 'i': case 'u': case 'x': case 'X': case 'o': case 'c':
			if (p - lenmod >= 2 && lenmod[0]=='l' && lenmod[1]=='l') n = snprintf(buf,sizeof buf,spec,va_arg(args,long long));
			else if (*lenmod=='l' || *lenmod=='z') n = snprintf(buf,sizeof buf,spec,va_arg(args,long));
			else n = snprintf(buf,sizeof buf,spec,va_arg(args,int));
			break;
		case 'f': case 'g': case 'e': n = snprintf(buf,sizeof buf,spec,va_arg(args,double)); break;
		case 's': n = snprintf(buf,sizeof buf,spec,va_arg(args,char*)); break;
		case 'p': n = snprintf(buf,sizeof buf,spec,va_arg(args,void*)); break;
		case '%': buf[0]='%'; buf[1]=0; n=1; break;
		default: buf[0]= conv?conv:0; buf[1]=0; n = conv?1:0; break;
		}
		if (n > 0) { fwrite(buf,1,(size_t)n,out); total += n; }
		if (conv) p++;
	}
	return total;
}

int vprintk(const char *fmt, va_list args)
{
	int level = 6;  /* alapért. INFO */
	const char *p = fmt;
	if (p[0] == '\001' && p[1]) {
		if (p[1] >= '0' && p[1] <= '7') level = p[1] - '0';
		else if (p[1] == 'c') level = 6;   /* KERN_CONT */
		p += 2;
	}
	/* MINDIG formázunk (a dmesg-puffernek, fix alapért. úton); csak a stderr-console szűrt. */
	char *mb = NULL; size_t msz = 0; FILE *mf = dmesg_path() ? open_memstream(&mb, &msz) : NULL;
	if (mf) {
		int r = uk_vformat(mf, p, args);
		fclose(mf);
		uk_dmesg_write(level, mb, msz);                  /* dmesg-fájl: szűretlen */
		if (level <= loglevel()) { fwrite(mb, 1, msz, stderr); fflush(stderr); }  /* console: szűrt */
		free(mb);
		return r;
	}
	/* nincs UK_DMESG (vagy open_memstream hiba) -> a régi, közvetlen, szűrt út */
	if (level > loglevel()) return 0;
	int r = uk_vformat(stderr, p, args);
	fflush(stderr);
	return r;
}

int printk(const char *fmt, ...)
{
	va_list ap; va_start(ap, fmt);
	int r = vprintk(fmt, ap);
	va_end(ap);
	return r;
}

int scnprintf(char *buf, size_t size, const char *fmt, ...)
{
	va_list ap; va_start(ap, fmt);
	int r = vsnprintf(buf, size, fmt, ap);
	va_end(ap);
	if (r < 0) return 0;
	return (size_t)r >= size ? (int)size - 1 : r;
}

void dump_stack(void) { fprintf(stderr, "uKernel: dump_stack() (userspace — nincs kernel-stack)\n"); }

unsigned long simple_strtoul(const char *cp, char **endp, unsigned int base)
{ return strtoul(cp, endp, base); }

long simple_strtol(const char *cp, char **endp, unsigned int base)
{ return strtol(cp, endp, base); }

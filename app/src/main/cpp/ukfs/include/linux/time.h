/* uKernel hamis <linux/time.h> — időzóna + alap idő-segédek. */
#ifndef _UK_LINUX_TIME_H
#define _UK_LINUX_TIME_H
#include <linux/types.h>
#include <linux/time64.h>

struct timezone { int tz_minuteswest; int tz_dsttime; };
extern struct timezone sys_tz;

#define SECS_PER_DAY	(24 * 60 * 60)
#define days_in_month(m) (0)

/* time64_to_tm: a Unix-másodpercet (+offset) bontja le év/hó/nap/óra/perc/mp-re a `struct tm`-be
 * (a FAT/exfat időkódolás — fat_time_unix2fat — ezt használja). VALÓDI impl a vfs.c-ben (libc
 * gmtime_r); a korábbi no-op stub a `tm`-et érintetlenül hagyta → tm_year=0 → MINDIG 1980. */
void time64_to_tm(time64_t totalsecs, int offset, void *result);
extern time64_t mktime64(unsigned int year, unsigned int mon, unsigned int day,
			 unsigned int hour, unsigned int min, unsigned int sec);
#endif

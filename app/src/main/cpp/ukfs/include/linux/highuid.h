/* uKernel hamis <linux/highuid.h> — a VALÓDI makrók (eddig no-op stubokra oldódtak, ezért az
 * ext4_fill_raw_inode 0-t írt a raw-inode uid/gid-jébe → a chown nem perzisztált). */
#ifndef _UK_LINUX_HIGHUID_H
#define _UK_LINUX_HIGHUID_H
#include <linux/types.h>
#define low_16_bits(x)	((x) & 0xFFFF)
#define high_16_bits(x)	(((x) & 0xFFFF0000) >> 16)
#define fs_high2lowuid(uid) ((uid) > 0xFFFF ? (uid_t)0xFFFF : (uid_t)(uid))
#define fs_high2lowgid(gid) ((gid) > 0xFFFF ? (gid_t)0xFFFF : (gid_t)(gid))
#endif

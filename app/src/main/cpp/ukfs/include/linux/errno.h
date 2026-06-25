/* uKernel hamis <linux/errno.h>.
 *
 * FONTOS: mivel a -I include a keresési út elején van, a glibc <errno.h> ->
 * <bits/errno.h> -> <linux/errno.h> lánc EZT a fejlécet találja meg a kernelé
 * helyett. Ezért itt MAGUNK definiáljuk a standard Linux errno-számokat
 * (asm-generic/errno-base + a gyakori kiterjesztett kódok), különben az
 * ENODEV/EINVAL/... sehol nem lenne definiálva. */
#ifndef _UK_LINUX_ERRNO_H
#define _UK_LINUX_ERRNO_H

#ifndef EPERM
#define EPERM    1
#define ENOENT   2
#define ESRCH    3
#define EINTR    4
#define EIO      5
#define ENXIO    6
#define E2BIG    7
#define ENOEXEC  8
#define EBADF    9
#define ECHILD  10
#define EAGAIN  11
#define ENOMEM  12
#define EACCES  13
#define EFAULT  14
#define ENOTBLK 15
#define EBUSY   16
#define EEXIST  17
#define EXDEV   18
#define ENODEV  19
#define ENOTDIR 20
#define EISDIR  21
#define EINVAL  22
#define ENFILE  23
#define EMFILE  24
#define ENOTTY  25
#define ETXTBSY 26
#define EFBIG   27
#define ENOSPC  28
#define ESPIPE  29
#define EROFS   30
#define EMLINK  31
#define EPIPE   32
#define EDOM    33
#define ERANGE  34
#endif

/* kiterjesztett, driverek által gyakran használt kódok */
#ifndef EDEADLK
#define EDEADLK      35
#define ENAMETOOLONG 36
#define ENOLCK       37
#define ENOSYS       38
#define ENOTEMPTY    39
#define ELOOP        40
#define ENOMSG       42
#define EPROTO       71
#define EOVERFLOW    75
#define EBADMSG      74
#define EILSEQ       84
#define ENODATA      61
#define ETIME        62
#define ENOLINK      67
#define ECOMM        70
#define EOPNOTSUPP   95
#define ECONNRESET  104
#define ENOBUFS     105
#define ETIMEDOUT   110
#define ECONNREFUSED 111
#define EALREADY    114
#define EINPROGRESS 115
#define EREMOTEIO   121
#define ECANCELED   125
#define ESHUTDOWN   108
#define EAFNOSUPPORT 97
#define ENOTSUPP    524
#define ERESTARTSYS 512
#define EPROBE_DEFER 517
#define ENOIOCTLCMD  515
#endif
#ifndef EHOSTUNREACH
#define EHOSTUNREACH 113
#endif
#ifndef EADDRNOTAVAIL
#define EADDRNOTAVAIL 99
#endif
#ifndef EADDRINUSE
#define EADDRINUSE   98
#endif
#ifndef ENETDOWN
#define ENETDOWN     100
#endif
#ifndef EL3RST
#define EL3RST       47
#endif
#ifndef EMSGSIZE
#define EMSGSIZE     90
#endif
#ifndef ETOOMANYREFS
#define ETOOMANYREFS 109
#endif
/* kernel-belső, a VFS/fs_parser használja */
#ifndef EIOCBQUEUED
#define EIOCBQUEUED 529
#endif
#ifndef ESTALE
#define ESTALE 116
#define EUCLEAN 117
#define EWOULDBLOCK 11
#endif
#ifndef EDQUOT
#define EDQUOT 122
#define ENOKEY 126
#endif
#ifndef EFSCORRUPTED
#define EFSCORRUPTED 117
#define EFSBADCRC 74
#define EFBIG 27
#endif
#ifndef ENOPARAM
#define ENOPARAM     519
#endif
#ifndef ERESTARTNOINTR
#define ERESTARTNOINTR 513
#define ERESTARTNOHAND 514
#define ERESTART_RESTARTBLOCK 516
#endif

/* IS_ERR / PTR_ERR — kernel pointer-hiba konvenció */
#define MAX_ERRNO       4095
#define IS_ERR_VALUE(x) ((unsigned long)(void *)(x) >= (unsigned long)-MAX_ERRNO)

static inline void *ERR_PTR(long e) { return (void *)e; }
static inline long PTR_ERR(const void *p) { return (long)p; }
static inline int IS_ERR(const void *p) { return IS_ERR_VALUE((unsigned long)p); }
static inline void *ERR_CAST(const void *p) { return (void *)p; }
static inline int PTR_ERR_OR_ZERO(const void *p){ return IS_ERR(p) ? (int)(long)p : 0; }
static inline int IS_ERR_OR_NULL(const void *p) { return !p || IS_ERR(p); }

#endif

/* uKernel hamis <linux/types.h> — userspace típus-leképezés. */
#ifndef _UK_LINUX_TYPES_H
#define _UK_LINUX_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <sys/types.h>

/* bionic's <sys/types.h> lacks the BSD `ulong` (glibc provides it under _GNU_SOURCE);
 * the ntfs3 driver uses it, so define it on bionic. */
#if defined(__BIONIC__)
typedef unsigned long ulong;
#endif

#ifndef __BITS_PER_LONG
#define __BITS_PER_LONG 64
#endif
#ifndef BITS_PER_LONG
#define BITS_PER_LONG 64
#endif

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
/* u64/s64 a kernel-konvenció szerint long long (különben az UAPI swab.h ütközik) */
typedef unsigned long long u64;
typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef long long s64;

typedef u8  __u8;
typedef u16 __u16;
typedef u32 __u32;
typedef u64 __u64;
typedef s8  __s8;
typedef s16 __s16;
typedef s32 __s32;
typedef s64 __s64;

/* endian-jelölt típusok — userspace-ben sima skalárok */
typedef u16 __le16;
typedef u16 __be16;
typedef u32 __le32;
typedef u32 __be32;
typedef u64 __le64;
typedef u64 __be64;
typedef u16 __sum16;
typedef u32 unicode_t;
typedef unsigned int vm_fault_t;
typedef unsigned long pgoff_t;
typedef u32 errseq_t;
typedef unsigned int fgf_t;
typedef u64 __attribute__((aligned(8))) __aligned_u64;
typedef u32 __attribute__((aligned(4))) __aligned_be32;
typedef u32 __wsum;

typedef __SIZE_TYPE__ __kernel_size_t;
typedef long __kernel_ssize_t;
typedef long __kernel_off_t;
typedef int __kernel_pid_t;
typedef unsigned int gfp_t;
typedef unsigned int fmode_t;
typedef unsigned short umode_t;
typedef u64 dma_addr_t;
#ifndef _UK_SECTOR_T
#define _UK_SECTOR_T
typedef u64 sector_t;
/* blkcnt_t a host <sys/types.h>-bol johet (glibc) — nem definialjuk ujra. */
#endif
/* loff_t a host <sys/types.h>-ból jön — nem definiáljuk újra. */

/* Korai globalis forward-deklaraciok: tobb driver-header deklaral fuggvenyeket
 * seq_file/file pointer-parameterrel, MIELOTT a teljes definicio elerheto;
 * enelkul param-listaban scope-olt incomplete tipus lenne (conflicting types). */
struct seq_file;
struct file;
struct inode;

struct list_head { struct list_head *next, *prev; };
struct hlist_head { struct hlist_node *first; };
struct hlist_node { struct hlist_node *next, **pprev; };

#ifndef _UK_RCU_HEAD
#define _UK_RCU_HEAD
struct rcu_head { struct rcu_head *next; void (*func)(struct rcu_head *head); };
typedef struct rcu_head callback_head;
#endif

typedef struct { int counter; } atomic_t;
typedef struct { long counter; } atomic64_t;

/* endian-konverzió: az asm/byteorder.h a KANONIKUS forrás (mindenki onnan kapja,
 * a wifi és a usb-serial is). A guard kezeli a kölcsönös includolást. */
#include <asm/byteorder.h>

#ifndef xchg
#define xchg(ptr, new) __atomic_exchange_n((ptr), (new), __ATOMIC_SEQ_CST)
#define cmpxchg(ptr, old, new) __sync_val_compare_and_swap((ptr), (old), (new))
#endif
/* try_cmpxchg(ptr, oldp, new): true ha *ptr==*oldp (és *ptr=new lesz), különben *oldp=*ptr.
 * MAKRÓ kell — különben az ext4 a no-op stubra (mindig false) oldódik → VÉGTELEN cmpxchg-loop! */
#ifndef try_cmpxchg
#define try_cmpxchg(ptr, oldp, new) __atomic_compare_exchange_n((ptr), (oldp), (new), 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)
#endif
#endif

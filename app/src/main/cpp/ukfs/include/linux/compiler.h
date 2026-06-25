/* uKernel hamis <linux/compiler.h> — attribútumok és barrierek userspace-ben. */
#ifndef _UK_LINUX_COMPILER_H
#define _UK_LINUX_COMPILER_H

#define __init
#define __exit
#define __user
#define __kernel
#define __iomem
#define __force
#define __must_check
#define __maybe_unused   __attribute__((unused))
#define __always_unused  __attribute__((unused))
#define __packed         __attribute__((packed))
#define __aligned(x)     __attribute__((aligned(x)))
#define __printf(a, b)   __attribute__((format(printf, a, b)))
#define __nonstring 
#define ____cacheline_aligned_in_smp 
#define __cacheline_aligned 
#define __randomize_layout 
#define __no_randomize_layout 
#define __read_mostly
#define __rcu
#define __percpu
#ifndef __cold
#define __cold           __attribute__((cold))
#ifndef noinline
#define noinline
#endif
#ifndef __noinline
#define __noinline
#endif
#ifndef noinline_for_stack
#define noinline_for_stack
#endif
#endif
#ifndef __hot
#define __hot            __attribute__((hot))
#endif
#ifndef __must_hold
#define __must_hold(x)
#endif
#define printk_index_subsys_emit(pfx, lvl, fmt, ...) do {} while (0)

/* A kernel a plain `inline`-t gnu_inline-ra (GNU89) képezi: a .c-ben definiált
 * inline függvények KÜLSŐ szimbólumot is emittálnak (pl. is_same_ess, rtw_sec_*),
 * különben más fordítási egységből hívva feloldatlanok maradnának. CSAK a
 * driver-buildhez (a saját shim-kódunk normál szemantikát vár). */
#ifdef UKERNEL_DRIVER_BUILD
#define inline inline __attribute__((gnu_inline))
#endif

#define likely(x)        __builtin_expect(!!(x), 1)
#define unlikely(x)      __builtin_expect(!!(x), 0)

#define barrier()        __asm__ __volatile__("" ::: "memory")
#define smp_mb()         __sync_synchronize()
#define smp_rmb()        __sync_synchronize()
#define smp_wmb()        __sync_synchronize()
#define mb()             __sync_synchronize()
#define WRITE_ONCE(x, v) (*(volatile typeof(x) *)&(x) = (v))
#define READ_ONCE(x)     (*(volatile typeof(x) *)&(x))

#undef unreachable
#define unreachable()    __builtin_unreachable()
#define fallthrough      __attribute__((fallthrough))

/* sparse/annotation no-op-ok */
#define __acquires(x)
#define __releases(x)
#define __acquire(x)     (void)0
#define __release(x)     (void)0
#define __cond_lock(x, c) (c)
#define __bitwise
#define __counted_by(m)
#define __counted_by_le(m)
#define __counted_by_be(m)

/* rugalmas tömb tagok unionban (linux/usb/cdc.h, garmin_gps.c) */
#define __DECLARE_FLEX_ARRAY(TYPE, NAME)	\
	struct { struct { } __empty_##NAME; TYPE NAME[]; }
#ifndef DECLARE_FLEX_ARRAY
#define DECLARE_FLEX_ARRAY(TYPE, NAME)  __DECLARE_FLEX_ARRAY(TYPE, NAME)
#endif

#ifndef noinline
#define noinline
#endif
#ifndef __noinline
#define __noinline
#endif
#ifndef noinline_for_stack
#define noinline_for_stack
#endif
#endif

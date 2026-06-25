/* uKernel hamis <linux/kernel.h> — alapmakrók és printk. */
#ifndef _UK_LINUX_KERNEL_H
#define _UK_LINUX_KERNEL_H

#include <linux/types.h>
#include <linux/compiler.h>
#include <stdarg.h>
#include <asm/unaligned.h>
#include <linux/ktime.h>

#define container_of(ptr, type, member) \
	((type *)((char *)(ptr) - offsetof(type, member)))

/* egész-típus határok — literál alak, hogy a #if kifejezésekben is működjenek
 * (a glibc <limits.h>/<stdint.h> ezeket szintén definiálhatja; #ifndef-fel
 * óvatosan, hogy ne ütközzünk pl. a wifi-buildbe behúzott rendszerfejlécekkel) */
#ifndef U8_MAX
#define U8_MAX		(0xff)
#define S8_MAX		(0x7f)
#define S8_MIN		(-S8_MAX - 1)
#define U16_MAX		(0xffff)
#define S16_MAX		(0x7fff)
#define S16_MIN		(-S16_MAX - 1)
#define U32_MAX		(0xffffffffU)
#define S32_MAX		(0x7fffffff)
#define S32_MIN		(-S32_MAX - 1)
#define U64_MAX		(0xffffffffffffffffULL)
#define S64_MAX		(0x7fffffffffffffffLL)
#define S64_MIN		(-S64_MAX - 1)
#endif
#ifndef UINT_MAX
#define UINT_MAX	(~0U)
#endif
#ifndef INT_MAX
#define INT_MAX		(0x7fffffff)
#endif
#ifndef INT_MIN
#define INT_MIN		(-INT_MAX - 1)
#endif
#ifndef USHRT_MAX
#define USHRT_MAX	(0xffff)
#endif
#ifndef SHRT_MAX
#define SHRT_MAX	(0x7fff)
#endif
#ifndef SHRT_MIN
#define SHRT_MIN	(-SHRT_MAX - 1)
#endif
#ifndef ULONG_MAX
#define ULONG_MAX	(~0UL)
#endif
#ifndef LONG_MAX
#define LONG_MAX	(__LONG_MAX__)
#endif
#ifndef LONG_MIN
#define LONG_MIN	(-LONG_MAX - 1)
#endif
#ifndef ULLONG_MAX
#define ULLONG_MAX	(~0ULL)
#endif
#ifndef LLONG_MAX
#define LLONG_MAX	(0x7fffffffffffffffLL)
#endif
#ifndef LLONG_MIN
#define LLONG_MIN	(-LLONG_MAX - 1)
#endif

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

/* IS_ENABLED/IS_BUILTIN — a CONFIG_* makró vagy 1-re definiált, vagy nem létezik.
 * A __is_defined trükk: a definiált "1" tokent "1 ," -ra cseréljük, majd a
 * harmadik argumentum kiválasztásával 1-et vagy 0-t kapunk. */
#define __ARG_PLACEHOLDER_1	0,
#define __take_second_arg(__ignored, val, ...)	val
#define __is_defined(x)			___is_defined(x)
#define ___is_defined(val)		____is_defined(__ARG_PLACEHOLDER_##val)
#define ____is_defined(arg1_or_junk)	__take_second_arg(arg1_or_junk 1, 0)
#define IS_ENABLED(option)		__is_defined(option)
#define IS_BUILTIN(option)		__is_defined(option)
#define IS_MODULE(option)		0
#define IS_REACHABLE(option)		__is_defined(option)

#undef min
#undef max
#define min(a, b) ({ typeof(a) _a = (a); typeof(b) _b = (b); _a < _b ? _a : _b; })
#define max(a, b) ({ typeof(a) _a = (a); typeof(b) _b = (b); _a > _b ? _a : _b; })
#define min_t(t, a, b) ((t)(a) < (t)(b) ? (t)(a) : (t)(b))
#define max_t(t, a, b) ((t)(a) > (t)(b) ? (t)(a) : (t)(b))
#define clamp(v, lo, hi) min(max(v, lo), hi)
#define clamp_val(v, lo, hi) ((v) < (lo) ? (lo) : ((v) > (hi) ? (hi) : (v)))
#define clamp_t(t, v, lo, hi) min_t(t, max_t(t, v, lo), hi)
/* csak a driver-buildhez — a saját shim a libc abs()-ot hasznalja */
#ifdef UKERNEL_DRIVER_BUILD
#define abs(x) ({ typeof(x) __x = (x); __x < 0 ? -__x : __x; })
#endif

#define PAGE_SIZE        4096
#define PAGE_SHIFT       12
#ifndef PAGE_MASK
#define PAGE_MASK (~(PAGE_SIZE - 1))
#endif
#define ALIGN(x, a)      (((x) + (a) - 1) & ~((a) - 1))
#define ALIGN_DOWN(x, a)  ((x) & ~((a) - 1))
#define IS_ALIGNED(x, a)  (((x) & ((typeof(x))(a) - 1)) == 0)
#ifndef round_down
#define round_down(x, y)  ((x) & ~((typeof(x))(y) - 1))
#endif
#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#define roundup(x, y)    ((((x) + (y) - 1) / (y)) * (y))
/* in_range(b, first, len): b a [first, first+len) tartományban van-e (ext4 extent-leképezés).
 * MAKRÓ kell — különben az ext4 implicit függvényhívásként no-op stubra oldódik (mindig hole)! */
#ifndef in_range
#define in_range(b, first, len) ((b) >= (first) && (b) < (first) + (len))
#endif
#ifndef round_up
#define round_up(x, y)   ((((x) - 1) | ((__typeof__(x))(y) - 1)) + 1)
#endif
#ifndef rounddown
#define rounddown(x, y)  ((x) - ((x) % (y)))
#endif
#ifndef umin
#define umin(a, b)       ((a) < (b) ? (a) : (b))
#define umax(a, b)       ((a) > (b) ? (a) : (b))
#endif

#define BIT(n)           (1UL << (n))
#define _RET_IP_ ((unsigned long)__builtin_return_address(0))
#define _THIS_IP_ ((unsigned long)0)
#ifndef BITS_PER_LONG
#define BITS_PER_LONG    (8 * (int)sizeof(long))
#endif
#define GENMASK(h, l)    (((~0UL) << (l)) & (~0UL >> (BITS_PER_LONG - 1 - (h))))

#define upper_32_bits(n) ((u32)(((n) >> 16) >> 16))
#define lower_32_bits(n) ((u32)((n) & 0xffffffff))

/* printk szintek — a tényleges leképezés a printk.c-ben. */
#define KERN_SOH   "\001"
#define KERN_EMERG   KERN_SOH "0"
#define KERN_ALERT   KERN_SOH "1"
#define KERN_CRIT    KERN_SOH "2"
#define KERN_ERR     KERN_SOH "3"
#define KERN_WARNING KERN_SOH "4"
#define KERN_NOTICE  KERN_SOH "5"
#define KERN_INFO    KERN_SOH "6"
#define KERN_DEBUG   KERN_SOH "7"
#define KERN_CONT    KERN_SOH "c"

int printk(const char *fmt, ...) __printf(1, 2);
int vprintk(const char *fmt, va_list args);
#ifndef _UK_VA_FORMAT
#define _UK_VA_FORMAT
struct va_format { const char *fmt; va_list *va; };
#endif

#define pr_emerg(fmt, ...)   printk(KERN_EMERG   fmt, ##__VA_ARGS__)
#define pr_alert(fmt, ...)   printk(KERN_ALERT   fmt, ##__VA_ARGS__)
#define pr_crit(fmt, ...)    printk(KERN_CRIT    fmt, ##__VA_ARGS__)
#define pr_err(fmt, ...)     printk(KERN_ERR     fmt, ##__VA_ARGS__)
#define pr_warn(fmt, ...)    printk(KERN_WARNING fmt, ##__VA_ARGS__)
#define pr_warning(fmt, ...) printk(KERN_WARNING fmt, ##__VA_ARGS__)
#define pr_notice(fmt, ...)  printk(KERN_NOTICE  fmt, ##__VA_ARGS__)
#define pr_info(fmt, ...)    printk(KERN_INFO    fmt, ##__VA_ARGS__)
#define pr_debug(fmt, ...)   printk(KERN_DEBUG   fmt, ##__VA_ARGS__)
#define pr_cont(fmt, ...)    printk(KERN_CONT    fmt, ##__VA_ARGS__)

void dump_stack(void);
unsigned int get_random_u32(void);
void get_random_bytes(void *buf, int nbytes);
static inline int in_interrupt(void){ return 0; }
static inline int in_atomic(void){ return 0; }
#define do_div(n, base) ({ unsigned long __rem = (unsigned long)(n) % (unsigned long)(base); (n) = (unsigned long)(n) / (unsigned long)(base); __rem; })
int snprintf(char *buf, size_t size, const char *fmt, ...) __printf(3, 4);
int scnprintf(char *buf, size_t size, const char *fmt, ...) __printf(3, 4);
int sprintf(char *buf, const char *fmt, ...) __printf(2, 3);
int sscanf(const char *buf, const char *fmt, ...) __attribute__((format(scanf, 2, 3)));

/* egyszerű szám-parszolók, amiket a driverek használnak */
unsigned long simple_strtoul(const char *cp, char **endp, unsigned int base);
long simple_strtol(const char *cp, char **endp, unsigned int base);

#define WARN_ON(cond)      ({ int __c = !!(cond); if (__c) printk(KERN_WARNING "WARN_ON: %s:%d\n", __FILE__, __LINE__); __c; })
#define WARN_ON_ONCE(cond) WARN_ON(cond)
#define WARN(cond, fmt, ...) ({ int __c = !!(cond); if (__c) printk(KERN_WARNING fmt, ##__VA_ARGS__); __c; })
#define BUG_ON(cond)       do { if (cond) { printk(KERN_EMERG "BUG: %s:%d\n", __FILE__, __LINE__); __builtin_trap(); } } while (0)
#define BUG()              BUG_ON(1)

/* swap(a,b) — két azonos típusú érték cseréje */
#ifndef swap
#define swap(a, b) do { typeof(a) __tmp = (a); (a) = (b); (b) = __tmp; } while (0)
#endif

/* BUILD_BUG_ON — fordítási idejű ellenőrzés; userspace shimben no-op */
#ifndef BUILD_BUG_ON
#define BUILD_BUG_ON(cond)        ((void)sizeof(char[1 - 2*!!(cond)]))
#endif
#ifndef BUILD_BUG_ON_ZERO
#define BUILD_BUG_ON_ZERO(cond)   (sizeof(struct { int:(-!!(cond)); }))
#endif
#ifndef BUILD_BUG_ON_MSG
#define BUILD_BUG_ON_MSG(cond, msg) BUILD_BUG_ON(cond)
#endif

#ifndef __stringify
#define __stringify_1(x...) #x
#define __stringify(x...)   __stringify_1(x)
#endif
#ifndef struct_size
#define struct_size(p, member, count) \
	(sizeof(*(p)) + (count) * sizeof(*(p)->member))
#define struct_size_t(type, member, count) \
	(sizeof(type) + (count) * sizeof(((type *)0)->member[0]))
#define flex_array_size(p, member, count) \
	((count) * sizeof(*(p)->member))
#define size_add(a, b) ((a) + (b))
#define size_mul(a, b) ((a) * (b))
#define size_sub(a, b) ((a) - (b))
#endif
#ifndef sizeof_field
#define sizeof_field(TYPE, MEMBER) sizeof((((TYPE *)0)->MEMBER))
#endif
#ifndef offsetofend
#define offsetofend(TYPE, MEMBER) \
	(offsetof(TYPE, MEMBER) + sizeof_field(TYPE, MEMBER))
#endif
#ifndef this_cpu_ptr
#define this_cpu_ptr(ptr) (ptr)
#endif
#ifndef get_cpu_ptr
#define get_cpu_ptr(ptr) (ptr)
#endif
#ifndef put_cpu_ptr
#define put_cpu_ptr(ptr) do { (void)(ptr); } while (0)
#endif

/* u64_stats — userspace shimben nem-atomi mezo */
struct u64_stats_sync { int dummy; };
static inline void u64_stats_init(struct u64_stats_sync *s) { (void)s; }
static inline unsigned u64_stats_update_begin_irqsave(struct u64_stats_sync *s) { (void)s; return 0; }
static inline void u64_stats_update_end_irqrestore(struct u64_stats_sync *s, unsigned f) { (void)s; (void)f; }
static inline void u64_stats_update_begin(struct u64_stats_sync *s) { (void)s; }
static inline void u64_stats_update_end(struct u64_stats_sync *s) { (void)s; }
static inline void u64_stats_add(u64 *p, u64 v) { *p += v; }
static inline void u64_stats_inc(u64 *p) { *p += 1; }

#endif
#ifndef DIV_ROUND_CLOSEST
#define DIV_ROUND_CLOSEST(x, d) (((x) + (d)/2) / (d))
#endif

/* system_state — a uas shutdown-utvonala olvassa; mindig RUNNING. */
#ifndef _UK_SYSTEM_STATE
#define _UK_SYSTEM_STATE
enum system_states {
	SYSTEM_BOOTING,
	SYSTEM_SCHEDULING,
	SYSTEM_RUNNING,
	SYSTEM_HALT,
	SYSTEM_POWER_OFF,
	SYSTEM_RESTART,
	SYSTEM_SUSPEND,
};
#define system_state	SYSTEM_RUNNING
#endif

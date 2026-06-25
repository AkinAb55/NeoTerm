/* uKernel hamis <linux/minmax.h>. */
#ifndef _UK_LINUX_MINMAX_H
#define _UK_LINUX_MINMAX_H
#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif
#define min_t(t, a, b) ((t)(a) < (t)(b) ? (t)(a) : (t)(b))
#define max_t(t, a, b) ((t)(a) > (t)(b) ? (t)(a) : (t)(b))
#define clamp(v, lo, hi) min(max(v, lo), hi)
#define clamp_t(t, v, lo, hi) min_t(t, max_t(t, v, lo), hi)
#define clamp_val(v, lo, hi) clamp(v, lo, hi)
#ifndef umin
#define umin(a, b) ((a) < (b) ? (a) : (b))
#define umax(a, b) ((a) > (b) ? (a) : (b))
#endif
#define swap(a, b) do { typeof(a) __t = (a); (a) = (b); (b) = __t; } while (0)
#endif

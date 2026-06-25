#ifndef _UK_LINUX_HRTIMER_H
#define _UK_LINUX_HRTIMER_H
#include <linux/types.h>
#include <linux/ktime.h>
enum hrtimer_restart { HRTIMER_NORESTART, HRTIMER_RESTART };
enum hrtimer_mode { HRTIMER_MODE_REL, HRTIMER_MODE_ABS, HRTIMER_MODE_REL_PINNED };
typedef int clockid_t;
#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC 1
#endif
struct hrtimer {
	enum hrtimer_restart (*function)(struct hrtimer *);
	ktime_t _softexpires;
};
static inline void hrtimer_init(struct hrtimer *t, clockid_t c, enum hrtimer_mode m) { (void)t;(void)c;(void)m; }
static inline void hrtimer_setup(struct hrtimer *t, enum hrtimer_restart (*fn)(struct hrtimer *), clockid_t c, enum hrtimer_mode m) { t->function = fn; (void)c;(void)m; }
static inline void hrtimer_start(struct hrtimer *t, ktime_t tim, enum hrtimer_mode m) { (void)t;(void)tim;(void)m; }
static inline int  hrtimer_cancel(struct hrtimer *t) { (void)t; return 0; }
static inline int  hrtimer_active(const struct hrtimer *t) { (void)t; return 0; }
static inline u64  hrtimer_forward_now(struct hrtimer *t, ktime_t i) { (void)t;(void)i; return 0; }
#endif

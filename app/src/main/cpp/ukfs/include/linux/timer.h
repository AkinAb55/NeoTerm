/* uKernel hamis <linux/timer.h> — időzítő szál ütemezi a callbackeket. */
#ifndef _UK_LINUX_TIMER_H
#define _UK_LINUX_TIMER_H

#include <linux/types.h>
#include <linux/jiffies.h>

struct timer_list {
	struct list_head entry;
	unsigned long    expires;
	void (*function)(struct timer_list *);
	unsigned long    data;        /* régi API kompat */
	void (*old_function)(unsigned long);
	int              pending;
};

void timer_setup(struct timer_list *t, void (*fn)(struct timer_list *), unsigned int flags);
void init_timer(struct timer_list *t);
int  mod_timer(struct timer_list *t, unsigned long expires);
void add_timer(struct timer_list *t);
int  del_timer(struct timer_list *t);
int  del_timer_sync(struct timer_list *t);
int  timer_pending(const struct timer_list *t);

/* régi setup_timer(t, fn, data) kompat */
#define from_timer(var, t, field) container_of(t, typeof(*var), field)
/* újabb név (6.16+): timer_container_of(var, t, field) */
#define timer_container_of(var, t, field) container_of(t, typeof(*var), field)
int  timer_shutdown_sync(struct timer_list *t);

#endif

#ifndef _UK_LINUX_INTERRUPT_H
#define _UK_LINUX_INTERRUPT_H
#include <linux/types.h>
struct tasklet_struct {
	void (*func)(unsigned long);          /* regi API */
	void (*callback)(struct tasklet_struct *); /* uj API (tasklet_setup) */
	unsigned long data;
};
static inline void tasklet_init(struct tasklet_struct *t, void (*f)(unsigned long), unsigned long d){ t->func=f; t->callback=0; t->data=d; }
static inline void tasklet_setup(struct tasklet_struct *t, void (*cb)(struct tasklet_struct *)){ t->callback=cb; t->func=0; t->data=0; }
static inline void tasklet_schedule(struct tasklet_struct *t){ if(t->callback)t->callback(t); else if(t->func)t->func(t->data); }
static inline void tasklet_kill(struct tasklet_struct *t){ (void)t; }
static inline void tasklet_hi_schedule(struct tasklet_struct *t){ tasklet_schedule(t); }
static inline void tasklet_enable(struct tasklet_struct *t){ (void)t; }
static inline void tasklet_disable(struct tasklet_struct *t){ (void)t; }

typedef int irqreturn_t;
#define IRQ_NONE 0
#define IRQ_HANDLED 1

#include <linux/kernel.h>
#define from_tasklet(var, callback_tasklet, tasklet_fieldname) \
	container_of(callback_tasklet, typeof(*(var)), tasklet_fieldname)
#endif

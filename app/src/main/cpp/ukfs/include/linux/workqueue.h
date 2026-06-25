/* uKernel hamis <linux/workqueue.h> — worker-szál(ak) futtatják a work_struct-okat. */
#ifndef _UK_LINUX_WORKQUEUE_H
#define _UK_LINUX_WORKQUEUE_H

#include <linux/types.h>
#include <linux/timer.h>

struct work_struct;
typedef void (*work_func_t)(struct work_struct *);

struct work_struct {
	struct list_head entry;
	work_func_t      func;
	int              pending;
	struct workqueue_struct *wq;
};

struct delayed_work {
	struct work_struct work;
	unsigned long      delay_jiffies;
	struct timer_list  timer;
};

struct workqueue_struct;

#define INIT_WORK(w, f)          do { (w)->func = (f); INIT_LIST_HEAD(&(w)->entry); (w)->pending = 0; (w)->wq = NULL; } while (0)
#define INIT_DELAYED_WORK(dw, f) do { INIT_WORK(&(dw)->work, (f)); } while (0)

struct workqueue_struct *create_singlethread_workqueue(const char *name);
struct workqueue_struct *create_workqueue(const char *name);
/* WQ flag-ek — a userspace shimben nem befolyasoljak a viselkedest. */
#ifndef WQ_MEM_RECLAIM
#define WQ_UNBOUND		(1 << 1)
#define WQ_FREEZABLE		(1 << 2)
#define WQ_MEM_RECLAIM		(1 << 3)
#define WQ_HIGHPRI		(1 << 4)
#define WQ_CPU_INTENSIVE	(1 << 5)
#define WQ_PERCPU		(1 << 6)
#endif

struct workqueue_struct *alloc_workqueue(const char *fmt, unsigned int flags, int max_active, ...);
void destroy_workqueue(struct workqueue_struct *wq);
void flush_workqueue(struct workqueue_struct *wq);

int  queue_work(struct workqueue_struct *wq, struct work_struct *w);
int  queue_delayed_work(struct workqueue_struct *wq, struct delayed_work *dw, unsigned long delay);
int  schedule_work(struct work_struct *w);            /* rendszer-wq */
int  schedule_delayed_work(struct delayed_work *dw, unsigned long delay);
int  cancel_work_sync(struct work_struct *w);
int  cancel_delayed_work(struct delayed_work *dw);
int  cancel_delayed_work_sync(struct delayed_work *dw);
void flush_work(struct work_struct *w);
bool flush_delayed_work(struct delayed_work *dw);
bool mod_delayed_work(struct workqueue_struct *wq, struct delayed_work *dw, unsigned long delay);

extern struct workqueue_struct *system_wq;
#define system_dfl_wq system_wq
extern struct workqueue_struct *system_long_wq;
extern struct workqueue_struct *system_freezable_wq;
extern struct workqueue_struct *system_bh_wq;
extern struct workqueue_struct *system_power_efficient_wq;

#define work_pending(w) ((w)->pending)
#define delayed_work_pending(w) work_pending(&(w)->work)
#define WORK_BUSY_PENDING	(1 << 0)
#define WORK_BUSY_RUNNING	(1 << 1)
unsigned int work_busy(struct work_struct *work);

/* from_work / container_of helper (>=6.10 work API) */
#include <linux/kernel.h>
#define from_work(var, work, field) container_of(work, typeof(*(var)), field)
#define work_to_void(w) ((void *)(w))

#define INIT_WORK_ONSTACK(w, f) INIT_WORK(w, f)
#define DECLARE_WORK(n, f) struct work_struct n = { .func = (f) }
#define DECLARE_DELAYED_WORK(n, f) struct delayed_work n = { .work = { .func = (f) } }

#endif

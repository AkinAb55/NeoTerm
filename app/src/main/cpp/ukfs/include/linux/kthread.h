/* uKernel hamis <linux/kthread.h> + sched-bitek — pthread fölött. */
#ifndef _UK_LINUX_KTHREAD_H
#define _UK_LINUX_KTHREAD_H

#include <linux/types.h>

struct task_struct;
struct completion;

struct task_struct *kthread_create(int (*fn)(void *), void *data, const char *namefmt, ...);
struct task_struct *kthread_run_fn(int (*fn)(void *), void *data, const char *name);
#define kthread_run(fn, data, ...) kthread_run_fn((fn), (data), "kthread")
void wake_up_process(struct task_struct *t);
int  kthread_stop(struct task_struct *t);
int  kthread_should_stop(void);
void kthread_complete_and_exit(struct completion *c, long code);
void complete_and_exit(struct completion *c, long code);

/* ütemezés */
void schedule(void);
long schedule_timeout(long timeout_jiffies);
long schedule_timeout_interruptible(long timeout_jiffies);
void cond_resched(void);
void yield(void);

#define set_current_state(s)  do {} while (0)
#define __set_current_state(s) do {} while (0)
#ifndef current
#include <linux/sched.h>  /* current = (&uk_current_task) */
#endif

#endif

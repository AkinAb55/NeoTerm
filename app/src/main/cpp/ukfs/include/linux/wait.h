/* uKernel hamis <linux/wait.h> — wait_queue pthread cond fölött. */
#ifndef _UK_LINUX_WAIT_H
#define _UK_LINUX_WAIT_H

#include <pthread.h>
#include <linux/jiffies.h>

/* feladat-állapotok (a valódi kernelben sched.h-ból; itt a wait.h hozza, mert a
 * prepare_to_wait()-et használó driverek erre számítanak) */
#ifndef TASK_RUNNING
#define TASK_RUNNING		0x0000
#define TASK_INTERRUPTIBLE	0x0001
#define TASK_UNINTERRUPTIBLE	0x0002
#endif

typedef struct wait_queue_head {
	pthread_mutex_t m;
	pthread_cond_t  c;
	int             inited;
} wait_queue_head_t;

void init_waitqueue_head(wait_queue_head_t *q);
void __wake_up_all(wait_queue_head_t *q);
/* a feltételt a hívó kódja ellenőrzi; mi a cond-változóra ébresztünk/várunk */
void __wait_on(wait_queue_head_t *q, unsigned ms);

#define wake_up(q)              __wake_up_all(q)
#define wake_up_all(q)          __wake_up_all(q)
#define wake_up_interruptible(q) __wake_up_all(q)

#ifndef set_current_state
#define set_current_state(s)    do { (void)(s); } while (0)
#endif
#ifndef __set_current_state
#define __set_current_state(s)  do { (void)(s); } while (0)
#endif
static inline int waitqueue_active(wait_queue_head_t *q) { (void)q; return 0; }

/* wait_event*: pollozó megvalósítás a feltételre (egyszerű, de helyes). */
#define wait_event(wq, condition) \
	do { while (!(condition)) __wait_on(&(wq), 10); } while (0)

#define wait_event_interruptible(wq, condition) \
	({ while (!(condition)) __wait_on(&(wq), 10); 0; })

#define wait_event_timeout(wq, condition, tmo) \
	({ unsigned long __t = (tmo); \
	   while (!(condition) && __t) { __wait_on(&(wq), 10); __t = __t > 10 ? __t - 10 : 0; } \
	   (condition) ? (__t ? __t : 1) : 0; })

#define wait_event_interruptible_timeout(wq, condition, tmo) \
	wait_event_timeout(wq, condition, tmo)

/* wait_queue_entry + DEFINE_WAIT/prepare/finish — stub (digi_acceleport.c) */
struct wait_queue_entry {
	void *private;
	int (*func)(struct wait_queue_entry *, unsigned, int, void *);
	struct list_head entry;
};
typedef struct wait_queue_entry wait_queue_entry_t;

#ifndef current
struct task_struct;
#include <linux/sched.h>  /* current = (&uk_current_task) */
#endif
#define DECLARE_WAITQUEUE(name, tsk) struct wait_queue_entry name = { .private = (void *)(tsk) }
#define DEFINE_WAIT(name) struct wait_queue_entry name = { 0 }
#define DEFINE_WAIT_FUNC(name, function) struct wait_queue_entry name = { .func = (function) }
#define init_wait(w) do { (void)(w); } while (0)

static inline void prepare_to_wait(wait_queue_head_t *q, wait_queue_entry_t *wait, int state)
{ (void)q; (void)wait; (void)state; }
static inline void finish_wait(wait_queue_head_t *q, wait_queue_entry_t *wait)
{ (void)q; (void)wait; }
static inline void add_wait_queue(wait_queue_head_t *q, wait_queue_entry_t *wait)
{ (void)q; (void)wait; }
static inline void remove_wait_queue(wait_queue_head_t *q, wait_queue_entry_t *wait)
{ (void)q; (void)wait; }

#endif

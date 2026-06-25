#ifndef _UK_LINUX_SCHED_H
#define _UK_LINUX_SCHED_H
#include <linux/kthread.h>
#include <linux/jiffies.h>
#include <linux/wait.h>
/* minimál task_struct + current — a fájlrendszerek a current->fs->umask-ot olvassák */
struct uk_fs_struct { int users; int umask; };
struct task_struct { struct uk_fs_struct *fs; unsigned int flags; char comm[16]; int pid; void *journal_info; struct io_context *io_context;
	/* kthread (shim/core/sched.c) — EGYESÍTVE, hogy a sched.c és az FS-kód UGYANAZT a struct-ot lássa
	 * (különben a clean-rebuild "redefinition of struct task_struct"-tal elhasal). tid = pthread_t tárolva. */
	unsigned long tid; int (*kfn)(void *); void *kdata; volatile int should_stop; int started; };
extern struct task_struct uk_current_task;
#ifndef current
#define current (&uk_current_task)
#endif
static inline int signal_pending(struct task_struct *t){ (void)t; return 0; }
static inline void allow_signal(int sig){ (void)sig; }
static inline void disallow_signal(int sig){ (void)sig; }
static inline void flush_signals(struct task_struct *t){ (void)t; }
#ifndef SIGTERM
#define SIGTERM 15
#endif
#ifndef SIGUSR1
#define SIGUSR1 10
#endif
#ifndef SIGUSR2
#define SIGUSR2 12
#endif
#ifndef SIGALRM
#define SIGALRM 14
#endif
struct pid;
static inline struct pid *find_vpid(int nr){ (void)nr; return (struct pid*)0; }
static inline int kill_pid(struct pid *pid, int sig, int priv){ (void)pid;(void)sig;(void)priv; return 0; }
static inline int rtw_get_pid(void){ return 0; }
#define MAX_SCHEDULE_TIMEOUT  (~0L)
#ifndef TASK_RUNNING
#define TASK_INTERRUPTIBLE 1
#define TASK_UNINTERRUPTIBLE 2
#define TASK_RUNNING 0
#endif
#ifndef set_current_state
#define set_current_state(s)   do { (void)(s); } while (0)
#define __set_current_state(s) do { (void)(s); } while (0)
#endif
#endif

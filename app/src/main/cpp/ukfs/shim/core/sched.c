/* uKernel — idő, időzítők, workqueue-k és kthread-ek pthread fölött. */
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>

/* A "current" task (a driverek a current->fs/comm/pid-et olvassák) — a CORE shimben definiálva, hogy MINDEN
 * modul (FS, wifi, tty) megkapja. Korábban a shim/fs/vfs.c-ben volt, ezért a wifi-stack (csak shim+cfg80211+
 * rtl) nem találta: "undefined symbol: uk_current_task". */
struct uk_fs_struct uk_init_fs = { .users = 1, .umask = 0022 };
struct task_struct uk_current_task = { .fs = &uk_init_fs };
#include <stdlib.h>
#include <string.h>

/* ===== idő ===== */
static unsigned long g_boot_ms;
static unsigned long now_ms(void)
{
	struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
	return (unsigned long)ts.tv_sec * 1000UL + ts.tv_nsec / 1000000UL;
}
__attribute__((constructor)) static void clock_init(void) { g_boot_ms = now_ms(); }
unsigned long ukernel_jiffies(void) { return now_ms() - g_boot_ms; }

void msleep(unsigned int ms)  { struct timespec t = { ms / 1000, (long)(ms % 1000) * 1000000L }; nanosleep(&t, NULL); }
unsigned long msleep_interruptible(unsigned int ms) { msleep(ms); return 0; }
void mdelay(unsigned int ms)  { msleep(ms); }
void udelay(unsigned int us)  { struct timespec t = { us / 1000000, (long)(us % 1000000) * 1000L }; nanosleep(&t, NULL); }
void ndelay(unsigned int ns)  { struct timespec t = { 0, (long)ns }; nanosleep(&t, NULL); }
void usleep_range(unsigned long min_us, unsigned long max_us) { udelay((unsigned)((min_us + max_us) / 2)); }

/* ===== időzítő manager (egy szál, rendezetlen lista) ===== */
static pthread_mutex_t tmr_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  tmr_cond = PTHREAD_COND_INITIALIZER;
static LIST_HEAD(tmr_list);
static pthread_t       tmr_thread;
static int             tmr_started;

static void *timer_main(void *arg)
{
	(void)arg;
	pthread_mutex_lock(&tmr_lock);
	for (;;) {
		unsigned long now = ukernel_jiffies();
		struct timer_list *t, *n, *fire = NULL;
		unsigned long next = now + 1000;
		list_for_each_entry_safe(t, n, &tmr_list, entry) {
			if (t->pending && time_after_eq(now, t->expires)) { fire = t; break; }
			if (t->pending && time_before(t->expires, next)) next = t->expires;
		}
		if (fire) {
			list_del_init(&fire->entry);
			fire->pending = 0;
			void (*fn)(struct timer_list *) = fire->function;
			pthread_mutex_unlock(&tmr_lock);
			if (fn) fn(fire);
			pthread_mutex_lock(&tmr_lock);
			continue;
		}
		struct timespec ts; clock_gettime(CLOCK_REALTIME, &ts);
		unsigned long wait = next > now ? next - now : 1;
		ts.tv_sec += wait / 1000; ts.tv_nsec += (long)(wait % 1000) * 1000000L;
		if (ts.tv_nsec >= 1000000000L) { ts.tv_sec++; ts.tv_nsec -= 1000000000L; }
		pthread_cond_timedwait(&tmr_cond, &tmr_lock, &ts);
	}
	return NULL;
}
static void timer_ensure_thread(void)
{
	if (!tmr_started) { tmr_started = 1; pthread_create(&tmr_thread, NULL, timer_main, NULL); }
}

void timer_setup(struct timer_list *t, void (*fn)(struct timer_list *), unsigned int flags)
{ (void)flags; memset(t, 0, sizeof(*t)); INIT_LIST_HEAD(&t->entry); t->function = fn; }
void init_timer(struct timer_list *t) { memset(t, 0, sizeof(*t)); INIT_LIST_HEAD(&t->entry); }

int mod_timer(struct timer_list *t, unsigned long expires)
{
	pthread_mutex_lock(&tmr_lock);
	timer_ensure_thread();
	int was = t->pending;
	t->expires = expires;
	if (!t->pending) { list_add_tail(&t->entry, &tmr_list); t->pending = 1; }
	pthread_cond_signal(&tmr_cond);
	pthread_mutex_unlock(&tmr_lock);
	return was;
}
void add_timer(struct timer_list *t) { mod_timer(t, t->expires); }
int del_timer(struct timer_list *t)
{
	pthread_mutex_lock(&tmr_lock);
	int was = t->pending;
	if (t->pending) { list_del_init(&t->entry); t->pending = 0; }
	pthread_mutex_unlock(&tmr_lock);
	return was;
}
int del_timer_sync(struct timer_list *t) { return del_timer(t); }
int timer_pending(const struct timer_list *t) { return t->pending; }

/* ===== workqueue ===== */
struct workqueue_struct {
	pthread_mutex_t lock;
	pthread_cond_t  cond;
	struct list_head queue;
	pthread_t       worker;
	int             stop;
	char            name[32];
};

static void *wq_worker(void *arg)
{
	struct workqueue_struct *wq = arg;
	pthread_mutex_lock(&wq->lock);
	while (!wq->stop) {
		if (list_empty(&wq->queue)) { pthread_cond_wait(&wq->cond, &wq->lock); continue; }
		struct work_struct *w = list_first_entry(&wq->queue, struct work_struct, entry);
		list_del_init(&w->entry);
		w->pending = 0;
		work_func_t fn = w->func;
		pthread_mutex_unlock(&wq->lock);
		if (fn) fn(w);
		pthread_mutex_lock(&wq->lock);
	}
	pthread_mutex_unlock(&wq->lock);
	return NULL;
}

struct workqueue_struct *alloc_workqueue(const char *fmt, unsigned int flags, int max_active, ...)
{
	(void)flags; (void)max_active;
	struct workqueue_struct *wq = calloc(1, sizeof(*wq));
	if (!wq) return NULL;
	pthread_mutex_init(&wq->lock, NULL);
	pthread_cond_init(&wq->cond, NULL);
	INIT_LIST_HEAD(&wq->queue);
	snprintf(wq->name, sizeof(wq->name), "%s", fmt ? fmt : "wq");
	pthread_create(&wq->worker, NULL, wq_worker, wq);
	return wq;
}
struct workqueue_struct *create_singlethread_workqueue(const char *name) { return alloc_workqueue(name, 0, 1); }
struct workqueue_struct *create_workqueue(const char *name) { return alloc_workqueue(name, 0, 1); }

void destroy_workqueue(struct workqueue_struct *wq)
{
	if (!wq) return;
	pthread_mutex_lock(&wq->lock);
	wq->stop = 1; pthread_cond_broadcast(&wq->cond);
	pthread_mutex_unlock(&wq->lock);
	pthread_join(wq->worker, NULL);
	free(wq);
}
void flush_workqueue(struct workqueue_struct *wq)
{
	/* primitív: várjuk meg, míg kiürül a sor */
	for (;;) {
		pthread_mutex_lock(&wq->lock);
		int empty = list_empty(&wq->queue);
		pthread_mutex_unlock(&wq->lock);
		if (empty) break;
		msleep(1);
	}
}

int queue_work(struct workqueue_struct *wq, struct work_struct *w)
{
	pthread_mutex_lock(&wq->lock);
	if (w->pending) { pthread_mutex_unlock(&wq->lock); return 0; }
	w->pending = 1; w->wq = wq;
	list_add_tail(&w->entry, &wq->queue);
	pthread_cond_signal(&wq->cond);
	pthread_mutex_unlock(&wq->lock);
	return 1;
}

/* rendszer-workqueue: a system_wq DEFINÍCIÓJA a net_compat.c-ben van (a workqueue.h extern-je); itt csak
 * használjuk (lazy alloc), különben "multiple definition" a linkben. */
static struct workqueue_struct *get_system_wq(void)
{ if (!system_wq) system_wq = alloc_workqueue("system", 0, 1); return system_wq; }
int schedule_work(struct work_struct *w) { return queue_work(get_system_wq(), w); }

/* delayed work: időzítő teszi a sorba */
static void dwork_timer_fn(struct timer_list *t)
{
	struct delayed_work *dw = container_of(t, struct delayed_work, timer);
	queue_work(dw->work.wq ? dw->work.wq : get_system_wq(), &dw->work);
}
int queue_delayed_work(struct workqueue_struct *wq, struct delayed_work *dw, unsigned long delay)
{
	dw->work.wq = wq;
	timer_setup(&dw->timer, dwork_timer_fn, 0);
	mod_timer(&dw->timer, ukernel_jiffies() + delay);
	return 1;
}
int schedule_delayed_work(struct delayed_work *dw, unsigned long delay)
{ return queue_delayed_work(get_system_wq(), dw, delay); }

int cancel_work_sync(struct work_struct *w)
{
	if (w->wq) { pthread_mutex_lock(&w->wq->lock); if (w->pending) { list_del_init(&w->entry); w->pending = 0; } pthread_mutex_unlock(&w->wq->lock); }
	return 0;
}
int cancel_delayed_work(struct delayed_work *dw) { del_timer(&dw->timer); return cancel_work_sync(&dw->work); }
int cancel_delayed_work_sync(struct delayed_work *dw) { return cancel_delayed_work(dw); }
void flush_work(struct work_struct *w) { if (w->wq) flush_workqueue(w->wq); }

/* ===== kthread ===== (a struct task_struct a <linux/sched.h>-ban van — egyesített FS+kthread) */
static void *kthread_trampoline(void *arg)
{ struct task_struct *t = arg; if (t->kfn) t->kfn(t->kdata); return NULL; }

struct task_struct *kthread_create(int (*fn)(void *), void *data, const char *namefmt, ...)
{
	(void)namefmt;
	struct task_struct *t = calloc(1, sizeof(*t));
	if (!t) return NULL;
	t->kfn = fn; t->kdata = data;
	return t;   /* wake_up_process indítja */
}
void wake_up_process(struct task_struct *t)
{ if (t && !t->started) { t->started = 1; pthread_t pt; pthread_create(&pt, NULL, kthread_trampoline, t); t->tid = (unsigned long)pt; } }
struct task_struct *kthread_run_fn(int (*fn)(void *), void *data, const char *name)
{ struct task_struct *t = kthread_create(fn, data, name); if (t) wake_up_process(t); return t; }

int kthread_should_stop(void) { return 0; }  /* a thread-lokális vizsgálatot a driver maga kezeli */
int kthread_stop(struct task_struct *t)
{ if (!t) return 0; t->should_stop = 1; if (t->started) pthread_join((pthread_t)t->tid, NULL); free(t); return 0; }

void schedule(void) { sched_yield(); }
long schedule_timeout(long j) { if (j > 0) msleep((unsigned)j); return 0; }
long schedule_timeout_interruptible(long j) { return schedule_timeout(j); }
void cond_resched(void) { sched_yield(); }
void yield(void) { sched_yield(); }

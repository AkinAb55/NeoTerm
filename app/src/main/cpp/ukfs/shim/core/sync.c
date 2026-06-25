/* uKernel — zárak és szinkronizáció pthread fölött. */
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/completion.h>
#include <linux/wait.h>
#include <linux/jiffies.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>

static void abstime_from_ms(struct timespec *ts, unsigned ms)
{
	clock_gettime(CLOCK_REALTIME, ts);
	ts->tv_sec  += ms / 1000;
	ts->tv_nsec += (long)(ms % 1000) * 1000000L;
	if (ts->tv_nsec >= 1000000000L) { ts->tv_sec++; ts->tv_nsec -= 1000000000L; }
}

/* --- spinlock (rekurzív mutex, hogy a userspace ütemezés ne akadjon be) --- */
static void ensure_spin(spinlock_t *l)
{
	if (!l->inited) {
		pthread_mutexattr_t a;
		pthread_mutexattr_init(&a);
		pthread_mutexattr_settype(&a, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(&l->m, &a);
		pthread_mutexattr_destroy(&a);
		l->inited = 1;
	}
}
void spin_lock_init(spinlock_t *l) { l->inited = 0; ensure_spin(l); }
void spin_lock(spinlock_t *l)   { ensure_spin(l); pthread_mutex_lock(&l->m); }
void spin_unlock(spinlock_t *l) { pthread_mutex_unlock(&l->m); }

/* --- mutex --- */
void mutex_init(struct mutex *m) { pthread_mutex_init(&m->m, NULL); m->inited = 1; }
static void ensure_mutex(struct mutex *m) { if (!m->inited) mutex_init(m); }
void mutex_lock(struct mutex *m)   { ensure_mutex(m); pthread_mutex_lock(&m->m); m->locked = 1; }
int  mutex_lock_interruptible(struct mutex *m) { mutex_lock(m); return 0; }
int  mutex_trylock(struct mutex *m){ ensure_mutex(m); int r = pthread_mutex_trylock(&m->m) == 0; if (r) m->locked = 1; return r; }
void mutex_unlock(struct mutex *m) { m->locked = 0; pthread_mutex_unlock(&m->m); }
int  mutex_is_locked(struct mutex *m) { return m && m->locked; }
void mutex_destroy(struct mutex *m){ if (m->inited) { pthread_mutex_destroy(&m->m); m->inited = 0; } }

/* --- semaphore --- */
void sema_init(struct semaphore *s, int val)
{ pthread_mutex_init(&s->m, NULL); pthread_cond_init(&s->c, NULL); s->count = val; }
void down(struct semaphore *s)
{ pthread_mutex_lock(&s->m); while (s->count <= 0) pthread_cond_wait(&s->c, &s->m); s->count--; pthread_mutex_unlock(&s->m); }
int down_interruptible(struct semaphore *s) { down(s); return 0; }
int down_trylock(struct semaphore *s)
{ int r; pthread_mutex_lock(&s->m); if (s->count > 0) { s->count--; r = 0; } else r = 1; pthread_mutex_unlock(&s->m); return r; }
void up(struct semaphore *s)
{ pthread_mutex_lock(&s->m); s->count++; pthread_cond_signal(&s->c); pthread_mutex_unlock(&s->m); }

void init_rwsem(struct rw_semaphore *s) { mutex_init(&s->lock); }

/* --- completion --- */
void init_completion(struct completion *x)
{ pthread_mutex_init(&x->m, NULL); pthread_cond_init(&x->c, NULL); x->done = 0; }
void reinit_completion(struct completion *x) { pthread_mutex_lock(&x->m); x->done = 0; pthread_mutex_unlock(&x->m); }

void complete(struct completion *x)
{ pthread_mutex_lock(&x->m); x->done++; pthread_cond_signal(&x->c); pthread_mutex_unlock(&x->m); }
void complete_all(struct completion *x)
{ pthread_mutex_lock(&x->m); x->done = (unsigned)-1 / 2; pthread_cond_broadcast(&x->c); pthread_mutex_unlock(&x->m); }

void wait_for_completion(struct completion *x)
{ pthread_mutex_lock(&x->m); while (!x->done) pthread_cond_wait(&x->c, &x->m); if (x->done != (unsigned)-1/2) x->done--; pthread_mutex_unlock(&x->m); }

unsigned long wait_for_completion_timeout(struct completion *x, unsigned long timeout_jiffies)
{
	struct timespec ts; abstime_from_ms(&ts, (unsigned)timeout_jiffies); /* HZ=1000 */
	int rc = 0;
	pthread_mutex_lock(&x->m);
	while (!x->done && rc == 0) rc = pthread_cond_timedwait(&x->c, &x->m, &ts);
	unsigned long left = 0;
	if (x->done) { if (x->done != (unsigned)-1/2) x->done--; left = 1; }
	pthread_mutex_unlock(&x->m);
	return left;  /* >0 = befejeződött, 0 = timeout */
}
int wait_for_completion_interruptible(struct completion *x) { wait_for_completion(x); return 0; }

/* --- wait queue --- */
static void ensure_wq(wait_queue_head_t *q)
{ if (!q->inited) { pthread_mutex_init(&q->m, NULL); pthread_cond_init(&q->c, NULL); q->inited = 1; } }
void init_waitqueue_head(wait_queue_head_t *q) { q->inited = 0; ensure_wq(q); }
void __wake_up_all(wait_queue_head_t *q) { ensure_wq(q); pthread_mutex_lock(&q->m); pthread_cond_broadcast(&q->c); pthread_mutex_unlock(&q->m); }
void __wait_on(wait_queue_head_t *q, unsigned ms)
{
	struct timespec ts; abstime_from_ms(&ts, ms);
	ensure_wq(q);
	pthread_mutex_lock(&q->m);
	pthread_cond_timedwait(&q->c, &q->m, &ts);
	pthread_mutex_unlock(&q->m);
}

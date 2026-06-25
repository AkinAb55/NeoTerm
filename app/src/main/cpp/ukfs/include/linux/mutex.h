/* uKernel hamis <linux/mutex.h> + semaphore — pthread fölött. */
#ifndef _UK_LINUX_MUTEX_H
#define _UK_LINUX_MUTEX_H

#include <linux/types.h>
#include <pthread.h>

struct mutex { pthread_mutex_t m; int inited; int locked; };

#define DEFINE_MUTEX(x) struct mutex x = { PTHREAD_MUTEX_INITIALIZER, 1, 0 }

/* mutex_is_locked: VALÓDI lock-állapot — a jbd2 BUG_ON(!mutex_is_locked(j_checkpoint_mutex))
 * assertje különben __builtin_trap-pel elszáll a commit-szálban. */
int mutex_is_locked(struct mutex *m);

void mutex_init(struct mutex *m);
void mutex_lock(struct mutex *m);
int  mutex_lock_interruptible(struct mutex *m);  /* 0 = sikeres */
int  mutex_trylock(struct mutex *m);   /* 1 = sikeres */
void mutex_unlock(struct mutex *m);
void mutex_destroy(struct mutex *m);

/* mutex_lock_io: a jbd2-commit ezzel zárolja a j_checkpoint_mutex-et. A mutex_lock DEKLARÁCIÓJA
 * UTÁN kell állnia (különben implicit-declaration hiba a -Wno-implicit nélküli buildekben). */
static inline void mutex_lock_io(struct mutex *m) { mutex_lock(m); }

/* ===== guard(mutex)(&lock) — scope-os automatikus unlock (cleanup attr). =====
 * A valódi <linux/cleanup.h> CLASS-gépezetét egyetlen mutex-osztályra szűkítjük,
 * mert a usb-serial bus.c / usb-serial.c csak ezt használja. */
struct __guard_mutex { struct mutex *lock; };
static inline struct __guard_mutex __guard_mutex_init(struct mutex *m)
{ mutex_lock(m); return (struct __guard_mutex){ .lock = m }; }
static inline void __guard_mutex_cleanup(struct __guard_mutex *g)
{ if (g->lock) mutex_unlock(g->lock); }

#define __guard_concat(a, b) a ## b
#define __guard_name(line) __guard_concat(__guard_var_, line)
#define guard(_name) \
	__attribute__((cleanup(__guard_##_name##_cleanup))) \
	struct __guard_##_name __guard_name(__LINE__) = __guard_##_name##_init

/* semaphore */
struct semaphore { pthread_mutex_t m; pthread_cond_t c; int count; };
void sema_init(struct semaphore *s, int val);
void down(struct semaphore *s);
int  down_interruptible(struct semaphore *s);
int  down_trylock(struct semaphore *s); /* 0 = sikeres */
void up(struct semaphore *s);

/* rw_semaphore — egyszerűsítve egy mutex */
struct rw_semaphore { struct mutex lock; };
void init_rwsem(struct rw_semaphore *s);
#define down_read(s)    mutex_lock(&(s)->lock)
#define up_read(s)      mutex_unlock(&(s)->lock)
#define down_write(s)   mutex_lock(&(s)->lock)
#define up_write(s)     mutex_unlock(&(s)->lock)

#endif

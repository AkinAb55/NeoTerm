#ifndef _UK_LINUX_LOCKDEP_H
#define _UK_LINUX_LOCKDEP_H
struct lock_class_key { int dummy; };
struct lockdep_map { int dummy; };
#define lockdep_set_class(lock, key) do {} while (0)
#define lockdep_init_map(a,b,c,d) do {} while (0)
#define lockdep_assert_held(l) do {} while (0)
#define rwsem_acquire(...) do{}while(0)
#define rwsem_acquire_read(...) do{}while(0)
#define rwsem_acquire_nest(...) do{}while(0)
#define rwsem_release(...) do{}while(0)
#define lock_acquire(...) do{}while(0)
#define lock_release(...) do{}while(0)
#define mutex_acquire(...) do{}while(0)
#define mutex_release(...) do{}while(0)
#define lock_map_acquire(...) do{}while(0)
#define lock_map_release(...) do{}while(0)
#define lock_map_acquire_read(...) do{}while(0)
#endif

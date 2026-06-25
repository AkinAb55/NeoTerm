#ifndef _UK_LINUX_RCUPDATE_H
#define _UK_LINUX_RCUPDATE_H
#include <linux/types.h>
#define rcu_dereference(p) (p)
#define rcu_dereference_protected(p, c) (p)
#define rcu_dereference_check(p, c) (p)
#define rcu_dereference_raw(p) (p)
#define rcu_assign_pointer(p, v) ((p) = (v))
#define RCU_INIT_POINTER(p, v) ((p) = (v))
#define rcu_read_lock() do {} while (0)
#define rcu_read_unlock() do {} while (0)
#define synchronize_rcu() do {} while (0)
#define kfree_rcu(ptr, rcu) kfree(ptr)
#define lockdep_is_held(l) 1
#define rcu_access_pointer(p) (p)
/* variadikus: a kernel 4. (cond) argumentumát elnyeljük */
#define list_for_each_entry_rcu(pos, head, member, ...) list_for_each_entry(pos, head, member)
#define hlist_for_each_entry_rcu(pos, head, member, ...) hlist_for_each_entry(pos, head, member)
#define __rcu
#endif

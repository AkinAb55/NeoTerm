#ifndef _UK_LINUX_RTNETLINK_H
#define _UK_LINUX_RTNETLINK_H
static inline void rtnl_lock(void){}
static inline void rtnl_unlock(void){}
static inline int rtnl_is_locked(void){ return 1; }
static inline int rtnl_trylock(void){ return 1; }
#endif

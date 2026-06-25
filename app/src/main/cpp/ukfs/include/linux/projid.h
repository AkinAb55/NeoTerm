#ifndef _UK_LINUX_PROJID_H
#define _UK_LINUX_PROJID_H
#include <linux/uidgid.h>
typedef unsigned int projid_t;
static inline kprojid_t make_kprojid(struct user_namespace *ns, projid_t projid) { (void)ns; return (kprojid_t){ .val = projid }; }
static inline projid_t from_kprojid(struct user_namespace *ns, kprojid_t k) { (void)ns; return k.val; }
static inline bool projid_valid(kprojid_t p) { return p.val != (projid_t)-1; }
#endif

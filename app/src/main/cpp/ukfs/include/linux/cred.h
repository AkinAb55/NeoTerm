/* uKernel hamis <linux/cred.h> — minden root. */
#ifndef _UK_LINUX_CRED_H
#define _UK_LINUX_CRED_H
#include <linux/uidgid.h>
struct cred { kuid_t uid, euid, fsuid; kgid_t gid, egid, fsgid; };
extern struct cred g_uk_cred;
static inline const struct cred *current_cred(void) { return &g_uk_cred; }
static inline kuid_t current_fsuid_cred(void) { return GLOBAL_ROOT_UID; }
#define get_current_user() NULL
#define current_user_ns() (&init_user_ns)
struct user_namespace;
extern struct user_namespace init_user_ns;
#endif

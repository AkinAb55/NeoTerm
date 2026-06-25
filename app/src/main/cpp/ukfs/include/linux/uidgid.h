/* uKernel hamis <linux/uidgid.h> — kuid_t/kgid_t userspace-stubok. */
#ifndef _UK_LINUX_UIDGID_H
#define _UK_LINUX_UIDGID_H
#include <linux/types.h>

typedef struct { uid_t val; } kuid_t;
typedef struct { gid_t val; } kgid_t;
typedef struct { unsigned int val; } kprojid_t;
#define KPROJIDT_INIT(v) (kprojid_t){ .val = (v) }
#define INVALID_PROJID KPROJIDT_INIT(-1)
typedef struct { unsigned int val; } vfsuid_t;
typedef struct { unsigned int val; } vfsgid_t;

#define KUIDT_INIT(v) (kuid_t){ .val = (v) }
#define KGIDT_INIT(v) (kgid_t){ .val = (v) }
#define GLOBAL_ROOT_UID KUIDT_INIT(0)
#define GLOBAL_ROOT_GID KGIDT_INIT(0)

static inline uid_t __kuid_val(kuid_t u) { return u.val; }
static inline gid_t __kgid_val(kgid_t g) { return g.val; }
static inline bool uid_eq(kuid_t a, kuid_t b) { return a.val == b.val; }
static inline bool gid_eq(kgid_t a, kgid_t b) { return a.val == b.val; }
static inline bool uid_valid(kuid_t u) { return u.val != (uid_t)-1; }
static inline bool gid_valid(kgid_t g) { return g.val != (gid_t)-1; }

struct user_namespace;
static inline uid_t from_kuid(struct user_namespace *ns, kuid_t u) { (void)ns; return u.val; }
static inline gid_t from_kgid(struct user_namespace *ns, kgid_t g) { (void)ns; return g.val; }
static inline kuid_t make_kuid(struct user_namespace *ns, uid_t u) { (void)ns; return KUIDT_INIT(u); }
static inline kgid_t make_kgid(struct user_namespace *ns, gid_t g) { (void)ns; return KGIDT_INIT(g); }
static inline uid_t from_kuid_munged(struct user_namespace *ns, kuid_t u) { (void)ns; return u.val; }
static inline gid_t from_kgid_munged(struct user_namespace *ns, kgid_t g) { (void)ns; return g.val; }
#endif

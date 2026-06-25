#ifndef _UK_LINUX_CLEANUP_H
#define _UK_LINUX_CLEANUP_H
#define __free(x)
#define __cleanup(x)
#define DEFINE_FREE(name, type, free)
#define DEFINE_CLASS(name, type, exit, init, ...)
#define DEFINE_GUARD(name, type, lock, unlock)
#define DEFINE_GUARD_COND(name, ext, condlock)
#define guard(name)
#define scoped_guard(name, ...) if (1)
#define CLASS(_name, var) struct fd var = uk_fd_get
#define no_free_ptr(p) (p)
#define return_ptr(p) return (p)
#endif

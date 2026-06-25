#ifndef _UK_LINUX_UTSNAME_H
#define _UK_LINUX_UTSNAME_H
struct new_utsname { char sysname[65], nodename[65], release[65], version[65], machine[65]; };
static inline struct new_utsname *utsname(void){ static struct new_utsname u = { "Linux","ukernel","6.6.0","uKernel","aarch64" }; return &u; }
#define init_utsname() utsname()
#endif

#ifndef _UK_LINUX_PERCPU_DEFS_H
#define _UK_LINUX_PERCPU_DEFS_H
#define DEFINE_PER_CPU(type, name) type name
#define DECLARE_PER_CPU(type, name) extern type name
#define this_cpu_ptr(ptr) (ptr)
#define per_cpu_ptr(ptr, cpu) (ptr)
#define get_cpu_var(var) (var)
#define put_cpu_var(var) do {} while (0)
#define this_cpu_inc(var) ((var)++)
#define this_cpu_dec(var) ((var)--)
#define this_cpu_read(var) (var)
#define this_cpu_add(var, n) ((var) += (n))
#define per_cpu(var, cpu) (var)
#define raw_cpu_ptr(ptr) (ptr)
#define __percpu
#endif

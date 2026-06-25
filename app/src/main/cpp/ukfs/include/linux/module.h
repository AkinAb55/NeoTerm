/* uKernel hamis <linux/module.h>.
 *
 * Kulcs: a module_init/module_exit makrók a .so betöltésekor (ELF konstruktor)
 * bejegyzik az init/exit függvényt a shim modul-registry-jébe. A uServer ezután
 * a ukernel_run_module_inits()-szel futtatja őket. Így a "dlopen driver.so →
 * a driver bejelentkezik" minta tisztán működik. */
#ifndef _UK_LINUX_MODULE_H
#define _UK_LINUX_MODULE_H

#include <linux/types.h>
#include <linux/kernel.h>

#ifndef KBUILD_MODNAME
#define KBUILD_MODNAME "ukmod"
#endif

/* a shim core/module.c-ben implementálva */
void ukernel_register_init(const char *mod, int (*fn)(void));
void ukernel_register_exit(const char *mod, void (*fn)(void));

#define module_init(fn) \
	__attribute__((constructor(110))) \
	static void __uk_ctor_init_##fn(void) { ukernel_register_init(KBUILD_MODNAME, (fn)); }

#define module_exit(fn) \
	__attribute__((constructor(111))) \
	static void __uk_ctor_exit_##fn(void) { ukernel_register_exit(KBUILD_MODNAME, (fn)); }

/* alszintű init-ek — egyszerűen ugyanaz a regisztráció */
#define subsys_initcall(fn)   module_init(fn)
#define device_initcall(fn)   module_init(fn)
#define late_initcall(fn)     module_init(fn)
#define core_initcall(fn)     module_init(fn)

/* metaadat-makrók — no-op (a sztringeket eldobjuk) */
#define MODULE_LICENSE(s)
#define MODULE_AUTHOR(s)
#define MODULE_DESCRIPTION(s)
#define MODULE_VERSION(s)
#define MODULE_ALIAS(s)
#define MODULE_ALIAS_FS(s)
#define MODULE_SOFTDEP(s)
#define MODULE_IMPORT_NS(s)
#define MODULE_FIRMWARE(s)
#define MODULE_DEVICE_TABLE(type, name)
#define MODULE_SUPPORTED_DEVICE(s)
#define MODULE_INFO(tag, info)
#define MODULE_ALIAS_CHARDEV_MAJOR(major)
#define MODULE_ALIAS_CHARDEV(major, minor)
#define MODULE_ALIAS_LDISC(ldisc)
#define MODULE_IMPORT_NS(ns)

#define THIS_MODULE ((struct module *)0)
struct module;

/* modul-paraméterek — no-op tárolás (a default érték marad) */
#define module_param(name, type, perm)
#define module_param_named(alias, name, type, perm)
#define module_param_array(name, type, nump, perm)
#define module_param_string(name, str, len, perm)
#define MODULE_PARM_DESC(var, desc)

/* kernel_param / kernel_param_ops — a usb-storage egyedi param-callbacket
 * (delay_use) hasznal; userspace shimben a regisztracio no-op. */
struct kernel_param {
	const char	*name;
	void		*arg;
};
struct kernel_param_ops {
	int (*set)(const char *val, const struct kernel_param *kp);
	int (*get)(char *buffer, const struct kernel_param *kp);
	void (*free)(void *arg);
};
#define module_param_cb(name, ops, arg, perm)

#define EXPORT_SYMBOL(sym)
#define EXPORT_SYMBOL_GPL(sym)

#define try_module_get(m)  (1)
#define module_put(m)      do {} while (0)

/* module_name(mod) — a valódi kernelben mod->name; itt a struct module opaque,
 * ezért fix nevet adunk vissza (usb-serial.c serial_proc_show). */
#define module_name(mod)   ((void)(mod), "usbserial")

#endif

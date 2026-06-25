/* uKernel hamis <linux/kobject.h> — a struct kobject/attribute/attribute_group a
 * device.h-ban van (a wifi azt használja); itt csak a kiegészítők (ext4 sysfs). */
#ifndef _UK_LINUX_KOBJECT_H
#define _UK_LINUX_KOBJECT_H
#include <linux/types.h>
#include <linux/list.h>
#include <linux/completion.h>
#include <linux/device.h>   /* struct kobject, attribute, attribute_group */
struct kobj_type {
	void (*release)(struct kobject *kobj);
	const struct sysfs_ops *sysfs_ops;
	const struct attribute_group **default_groups;
};
struct kset { struct list_head list; struct kobject kobj; };
struct sysfs_ops { ssize_t (*show)(struct kobject *, struct attribute *, char *); ssize_t (*store)(struct kobject *, struct attribute *, const char *, size_t); };
static inline int kobject_init_and_add(struct kobject *kobj, const struct kobj_type *t, struct kobject *parent, const char *fmt, ...) { (void)kobj;(void)t;(void)parent;(void)fmt; return 0; }
static inline void kobject_del(struct kobject *kobj) { (void)kobj; }
static inline void kobject_put(struct kobject *kobj) { (void)kobj; }
static inline struct kobject *kobject_get(struct kobject *kobj) { return kobj; }
static inline int kobject_uevent(struct kobject *kobj, int action) { (void)kobj;(void)action; return 0; }
#define KOBJ_CHANGE 3
extern struct kobject *fs_kobj;
/* nem-NULL dummy: az ext4_init_sysfs -ENOMEM-et adna NULL-ra; a kobject-műveletek no-op-ok */
static inline struct kobject *kobject_create_and_add(const char *name, struct kobject *parent){(void)name;(void)parent; static struct kobject uk_dummy_kobj; return &uk_dummy_kobj;}
#define ATTRIBUTE_GROUPS(_name) static const struct attribute_group _name##_group = { .attrs = _name##_attrs }; static const struct attribute_group *_name##_groups[] = { &_name##_group, NULL }
#endif

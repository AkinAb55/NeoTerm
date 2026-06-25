/* uKernel hamis <linux/string_choices.h> — a usb-storage altal hasznalt
 * str_*() segedek (debug-uzenetekhez). */
#ifndef _UK_LINUX_STRING_CHOICES_H
#define _UK_LINUX_STRING_CHOICES_H

#include <linux/types.h>

static inline const char *str_read_write(bool v) { return v ? "read" : "write"; }
#define str_write_read(v)	str_read_write(!(v))
static inline const char *str_on_off(bool v) { return v ? "on" : "off"; }
#define str_off_on(v)		str_on_off(!(v))
static inline const char *str_yes_no(bool v) { return v ? "yes" : "no"; }
#define str_no_yes(v)		str_yes_no(!(v))
static inline const char *str_enable_disable(bool v) { return v ? "enable" : "disable"; }
static inline const char *str_enabled_disabled(bool v) { return v ? "enabled" : "disabled"; }
static inline const char *str_true_false(bool v) { return v ? "true" : "false"; }

#endif

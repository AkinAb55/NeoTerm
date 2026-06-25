/* uKernel — struct device segédfüggvények. */
#include <linux/device.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

static char g_name_pool[64][48];
static int  g_name_idx;

const char *dev_name(const struct device *d)
{ return d && d->init_name ? d->init_name : "uk-dev"; }

int dev_set_name(struct device *d, const char *fmt, ...)
{
	char *buf = g_name_pool[g_name_idx++ % 64];
	va_list ap; va_start(ap, fmt);
	vsnprintf(buf, 48, fmt, ap);
	va_end(ap);
	d->init_name = buf;
	return 0;
}

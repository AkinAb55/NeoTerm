#ifndef _UK_LINUX_LIST_SORT_H
#define _UK_LINUX_LIST_SORT_H
#include <linux/list.h>
void list_sort(void *priv, struct list_head *head, int (*cmp)(void *, const struct list_head *, const struct list_head *));
#endif

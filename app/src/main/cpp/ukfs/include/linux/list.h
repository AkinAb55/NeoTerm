/* uKernel hamis <linux/list.h> — a klasszikus kernel listamakrók. */
#ifndef _UK_LINUX_LIST_H
#define _UK_LINUX_LIST_H

#include <linux/types.h>
#include <linux/kernel.h>   /* container_of */

#define LIST_HEAD_INIT(n) { &(n), &(n) }
#define LIST_HEAD(n) struct list_head n = LIST_HEAD_INIT(n)

static inline void INIT_LIST_HEAD(struct list_head *l) { l->next = l; l->prev = l; }

static inline void __list_add(struct list_head *n, struct list_head *p, struct list_head *nx)
{ nx->prev = n; n->next = nx; n->prev = p; p->next = n; }

static inline void list_add(struct list_head *n, struct list_head *h) { __list_add(n, h, h->next); }
static inline void list_add_tail(struct list_head *n, struct list_head *h) { __list_add(n, h->prev, h); }

static inline void __list_del(struct list_head *p, struct list_head *nx) { nx->prev = p; p->next = nx; }
static inline void list_del(struct list_head *e) { __list_del(e->prev, e->next); e->next = NULL; e->prev = NULL; }
static inline void list_del_init(struct list_head *e) { __list_del(e->prev, e->next); INIT_LIST_HEAD(e); }

static inline int list_empty(const struct list_head *h) { return h->next == h; }

static inline void list_move_tail(struct list_head *e, struct list_head *h)
{ __list_del(e->prev, e->next); list_add_tail(e, h); }

static inline void list_move(struct list_head *e, struct list_head *h)
{ __list_del(e->prev, e->next); list_add(e, h); }

static inline void list_splice(const struct list_head *list, struct list_head *head)
{
	if (list_empty(list)) return;
	struct list_head *first = list->next, *last = list->prev, *at = head->next;
	first->prev = head; head->next = first;
	last->next = at; at->prev = last;
}
static inline void list_splice_tail(struct list_head *list, struct list_head *head)
{
	if (list_empty(list)) return;
	struct list_head *first = list->next, *last = list->prev, *at = head->prev;
	first->prev = at; at->next = first;
	last->next = head; head->prev = last;
}
static inline void list_splice_init(struct list_head *list, struct list_head *head)
{ list_splice(list, head); INIT_LIST_HEAD(list); }
static inline void list_splice_tail_init(struct list_head *list, struct list_head *head)
{ list_splice_tail(list, head); INIT_LIST_HEAD(list); }

#define list_entry(ptr, type, member)  container_of(ptr, type, member)
#define list_first_entry(ptr, type, member) list_entry((ptr)->next, type, member)
#define list_next_entry(pos, member) \
	list_entry((pos)->member.next, typeof(*(pos)), member)

#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)

#define list_for_each_entry(pos, head, member) \
	for (pos = list_entry((head)->next, typeof(*pos), member); \
	     &pos->member != (head); \
	     pos = list_entry(pos->member.next, typeof(*pos), member))

#define list_for_each_entry_safe(pos, n, head, member) \
	for (pos = list_entry((head)->next, typeof(*pos), member), \
	     n = list_entry(pos->member.next, typeof(*pos), member); \
	     &pos->member != (head); \
	     pos = n, n = list_entry(n->member.next, typeof(*n), member))

/* ===== hlist (a rhashtable/sta_info használja) ===== */
#define HLIST_HEAD_INIT { .first = NULL }
static inline void INIT_HLIST_HEAD(struct hlist_head *h) { h->first = NULL; }
static inline void INIT_HLIST_NODE(struct hlist_node *n) { n->next = NULL; n->pprev = NULL; }
static inline int hlist_empty(const struct hlist_head *h) { return !h->first; }
static inline void hlist_add_head(struct hlist_node *n, struct hlist_head *h)
{ n->next = h->first; if (h->first) h->first->pprev = &n->next; h->first = n; n->pprev = &h->first; }
static inline void hlist_del(struct hlist_node *n)
{ if (n->next) n->next->pprev = n->pprev; if (n->pprev) *n->pprev = n->next; }
static inline void hlist_del_init(struct hlist_node *n) { if (n->pprev) { hlist_del(n); INIT_HLIST_NODE(n); } }
#define hlist_add_head_rcu(n, h) hlist_add_head(n, h)
#define hlist_del_rcu(n)         hlist_del(n)
#define hlist_del_init_rcu(n)    hlist_del_init(n)
#define hlist_entry(ptr, type, member) container_of(ptr, type, member)
#define hlist_entry_safe(ptr, type, member) ((ptr) ? hlist_entry(ptr, type, member) : NULL)
#define hlist_for_each(pos, head) for (pos = (head)->first; pos; pos = pos->next)
#define hlist_for_each_safe(pos, n, head) \
	for (pos = (head)->first; pos && ({ n = pos->next; 1; }); pos = n)
#define hlist_for_each_entry(pos, head, member) \
	for (pos = hlist_entry_safe((head)->first, typeof(*pos), member); pos; \
	     pos = hlist_entry_safe((pos)->member.next, typeof(*pos), member))
#define hlist_for_each_entry_safe(pos, n, head, member) \
	for (pos = hlist_entry_safe((head)->first, typeof(*pos), member); \
	     pos && ({ n = (pos)->member.next; 1; }); \
	     pos = hlist_entry_safe(n, typeof(*pos), member))
#define hlist_for_each_entry_rcu(pos, head, member) hlist_for_each_entry(pos, head, member)

#define list_last_entry(ptr, type, member) list_entry((ptr)->prev, type, member)
#endif

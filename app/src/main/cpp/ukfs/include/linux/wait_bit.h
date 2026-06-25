#ifndef _UK_LINUX_WAIT_BIT_H
#define _UK_LINUX_WAIT_BIT_H
#include <linux/wait.h>
#include <linux/sched.h>
struct wait_bit_key { unsigned long *flags; int bit_nr; unsigned long timeout; };
struct wait_bit_queue_entry { struct wait_bit_key key; struct wait_queue_entry wq_entry; };
int wake_bit_function(struct wait_queue_entry *wq_entry, unsigned mode, int sync, void *key);
struct wait_queue_head *bit_waitqueue(unsigned long *word, int bit);
#define __WAIT_BIT_KEY_INITIALIZER(word, bit) { .flags = word, .bit_nr = bit, }
#define DEFINE_WAIT_BIT(name, word, bit) \
	struct wait_bit_queue_entry name = { .key = __WAIT_BIT_KEY_INITIALIZER(word, bit), \
		.wq_entry = { .private = current, .func = wake_bit_function, .entry = LIST_HEAD_INIT((name).wq_entry.entry), } }
static inline void wait_on_bit(unsigned long *word, int bit, unsigned mode) { (void)word;(void)bit;(void)mode; }
static inline void wake_up_bit(unsigned long *word, int bit) { (void)word;(void)bit; }
static inline void clear_and_wake_up_bit(int bit, unsigned long *word) { (void)bit;(void)word; }
#endif

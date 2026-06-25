#ifndef _UK_LINUX_RFKILL_H
#define _UK_LINUX_RFKILL_H
#include <linux/types.h>
struct device;
enum rfkill_type { RFKILL_TYPE_ALL=0, RFKILL_TYPE_WLAN, RFKILL_TYPE_BLUETOOTH, RFKILL_TYPE_WWAN, NUM_RFKILL_TYPES };
struct rfkill;
struct rfkill_ops {
	void (*poll)(struct rfkill *rfkill, void *data);
	void (*query)(struct rfkill *rfkill, void *data);
	int  (*set_block)(void *data, bool blocked);
};
static inline struct rfkill *rfkill_alloc(const char *name, struct device *parent, enum rfkill_type type, const struct rfkill_ops *ops, void *ops_data) { (void)name;(void)parent;(void)type;(void)ops;(void)ops_data; return (struct rfkill *)1; }
static inline int  rfkill_register(struct rfkill *rfkill) { (void)rfkill; return 0; }
static inline void rfkill_unregister(struct rfkill *rfkill) { (void)rfkill; }
static inline void rfkill_destroy(struct rfkill *rfkill) { (void)rfkill; }
static inline bool rfkill_set_hw_state(struct rfkill *rfkill, bool blocked) { (void)rfkill; return blocked; }
static inline void rfkill_set_sw_state(struct rfkill *rfkill, bool blocked) { (void)rfkill;(void)blocked; }
static inline bool rfkill_blocked(struct rfkill *rfkill) { (void)rfkill; return false; }
#endif

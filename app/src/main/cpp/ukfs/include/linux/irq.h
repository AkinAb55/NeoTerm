#ifndef _UK_LINUX_IRQ_H
#define _UK_LINUX_IRQ_H
#include <linux/types.h>
struct irq_data { unsigned int irq; void *chip_data; unsigned long hwirq; };
struct irq_chip {
	const char *name;
	void (*irq_enable)(struct irq_data *data);
	void (*irq_disable)(struct irq_data *data);
	void (*irq_ack)(struct irq_data *data);
	void (*irq_mask)(struct irq_data *data);
	void (*irq_unmask)(struct irq_data *data);
	int  (*irq_set_type)(struct irq_data *data, unsigned int type);
	void (*irq_bus_lock)(struct irq_data *data);
	void (*irq_bus_sync_unlock)(struct irq_data *data);
	unsigned long flags;
};
struct irq_domain { void *host_data; };
struct irq_domain_ops {
	int  (*map)(struct irq_domain *, unsigned int, unsigned long);
	void (*unmap)(struct irq_domain *, unsigned int);
};
#define IRQCHIP_MASK_ON_SUSPEND 0
#define IRQ_TYPE_LEVEL_LOW 0x8
static inline void *irq_data_get_irq_chip_data(struct irq_data *d){ return d ? d->chip_data : 0; }
static inline unsigned int irqd_to_hwirq(struct irq_data *d){ return d ? d->hwirq : 0; }

struct irq_desc;
typedef unsigned long irq_hw_number_t;
typedef void (*irq_flow_handler_t)(struct irq_desc *desc);
extern struct irq_chip dummy_irq_chip;
static inline void irq_set_chip_and_handler(unsigned int irq, struct irq_chip *chip, irq_flow_handler_t handle) { (void)irq;(void)chip;(void)handle; }
static inline void irq_set_chip_data(unsigned int irq, void *data) { (void)irq;(void)data; }
static inline void handle_simple_irq(struct irq_desc *desc) { (void)desc; }
static inline int  generic_handle_irq(unsigned int irq) { (void)irq; return 0; }
static inline void handle_nested_irq(unsigned int irq) { (void)irq; }
#endif

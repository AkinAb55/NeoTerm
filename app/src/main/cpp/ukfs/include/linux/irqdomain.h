#ifndef _UK_LINUX_IRQDOMAIN_H
#define _UK_LINUX_IRQDOMAIN_H
#include <linux/types.h>
#include <linux/irq.h>
struct irq_domain;
struct fwnode_handle;
extern const struct irq_domain_ops irq_domain_simple_ops;
static inline struct fwnode_handle *irq_domain_alloc_named_fwnode(const char *name) { (void)name; return (struct fwnode_handle *)1; }
static inline void irq_domain_free_fwnode(struct fwnode_handle *fwnode) { (void)fwnode; }
static inline struct irq_domain *irq_domain_create_linear(struct fwnode_handle *fwnode, unsigned int size, const struct irq_domain_ops *ops, void *host_data) { (void)fwnode;(void)size;(void)ops;(void)host_data; return (struct irq_domain *)1; }
static inline void irq_domain_remove(struct irq_domain *d) { (void)d; }
static inline unsigned int irq_create_mapping(struct irq_domain *d, unsigned long hwirq) { (void)d;(void)hwirq; return 1; }
static inline unsigned int irq_find_mapping(struct irq_domain *d, unsigned long hwirq) { (void)d;(void)hwirq; return 1; }
static inline void irq_dispose_mapping(unsigned int virq) { (void)virq; }
#endif

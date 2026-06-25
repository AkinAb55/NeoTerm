#ifndef _UK_LINUX_VMALLOC_H
#define _UK_LINUX_VMALLOC_H
#include <linux/slab.h>
#ifndef VM_MAP
#define VM_MAP 0x00000004
#define VM_ALLOC 0x00000002
void *vmap(struct page **pages, unsigned int count, unsigned long flags, int prot);
void vunmap(const void *addr);
#endif
#define PAGE_KERNEL 0
#define PAGE_KERNEL_RO 0
typedef unsigned long pgprot_t;
#endif

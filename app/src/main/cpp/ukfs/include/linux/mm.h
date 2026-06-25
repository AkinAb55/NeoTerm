#ifndef _UK_LINUX_MM_H
#define _UK_LINUX_MM_H
#include <linux/types.h>
#include <linux/slab.h>   /* a lap-allokatorok (alloc_pages stb.) itt vannak */
#ifndef PAGE_MASK
#define PAGE_MASK (~(PAGE_SIZE-1))
#endif
static inline struct page *virt_to_page(const void *a) { return (struct page *)a; }
static inline void put_page(struct page *p) { (void)p; }
static inline void get_page(struct page *p) { (void)p; }

#ifndef _UK_VM_FAULT
#define _UK_VM_FAULT
struct vm_area_struct { struct file *vm_file; unsigned long vm_flags; struct address_space *vm_mapping; };
struct vm_fault { struct vm_area_struct *vma; unsigned long address; struct folio *folio; void *page; unsigned int flags; };
struct vm_operations_struct {
	int (*fault)(struct vm_fault *vmf);
	int (*page_mkwrite)(struct vm_fault *vmf);
	void (*map_pages)(struct vm_fault *vmf, unsigned long start, unsigned long end);
};
int filemap_fault(struct vm_fault *vmf);
void filemap_map_pages(struct vm_fault *vmf, unsigned long start, unsigned long end);
#endif

enum zone_stat_item { NR_FREE_PAGES, NR_ZONE_LRU_BASE };
long global_zone_page_state(enum zone_stat_item item);
unsigned long thp_get_unmapped_area(struct file *filp, unsigned long addr, unsigned long len, unsigned long pgoff, unsigned long flags);
/* virt_to_folio: a címet TARTALMAZÓ folio (a kernelben a page-leíró) — eddig NULL-stub volt, ami
 * a jbd2 fagyasztott-adat/escape ágán (new_folio = virt_to_folio(b_frozen_data)) NULL b_folio-t →
 * crc32c(NULL) crash-t okozott (csak a lassú USB-commit fagyasztó-ágán). VALÓDI impl a vfs.c-ben. */
struct folio *virt_to_folio(const void *x);
#endif

/* uKernel hamis <linux/slab.h> — kmalloc & co. a malloc család fölött. */
#ifndef _UK_LINUX_SLAB_H
#define _UK_LINUX_SLAB_H

#include <linux/types.h>

#define GFP_KERNEL   0x01
#define GFP_ATOMIC   0x02
#define GFP_DMA      0x04
#define GFP_NOWAIT   0x08
#define GFP_NOIO     0x10
#define GFP_KERNEL_ACCOUNT 0x20
#define __GFP_ZERO   0x100
#define __GFP_NOWARN 0x200
#define __GFP_COMP   0x400
#define __GFP_DMA    0x800
#define __GFP_HIGH   0x1000
#define __GFP_NOFAIL 0x2000
#define __GFP_RECLAIM 0x4000
#define GFP_USER     0x40
#define GFP_DMA32    0x80

void *kmalloc(size_t size, gfp_t flags);
void *kzalloc(size_t size, gfp_t flags);
void *kcalloc(size_t n, size_t size, gfp_t flags);
void *krealloc(void *p, size_t size, gfp_t flags);
void *kmemdup(const void *src, size_t len, gfp_t flags);
void  kfree(const void *p);

void *vmalloc(size_t size);
void *vzalloc(size_t size);
void  vfree(const void *p);

/* lap-allokatorok (a kernelben mm.h/gfp.h hozza; itt a slab.h, mert sok driver
 * csak a slab.h-t includolja) */
#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif
#ifndef PAGE_SHIFT
#define PAGE_SHIFT 12
#endif
struct page;
static inline void *page_address(const struct page *p) { return (void *)p; }
static inline unsigned int get_order(unsigned long size) { unsigned int o = 0; unsigned long n = (size + PAGE_SIZE - 1) >> PAGE_SHIFT; while (n > 1) { n = (n + 1) >> 1; o++; } return o; }
static inline struct page *alloc_pages(gfp_t gfp, unsigned int order) { return (struct page *)kzalloc(PAGE_SIZE << order, gfp); }
#define alloc_page(gfp) alloc_pages((gfp), 0)
static inline void __free_pages(struct page *page, unsigned int order) { (void)order; kfree(page); }
#define __free_page(page) __free_pages((page), 0)
/* __get_free_pages: LAPIGAZÍTOTT memória (a kernel valódi szemantikája) — a jbd2_alloc
 * BUG_ON(ptr & (size-1))-et vár (4096-blokkos ext4 journal-puffer), a kzalloc/malloc viszont csak
 * 16-igazított → jbd2 BUG_ON. posix_memalign-nal a (PAGE_SIZE<<order)-igazítás teljesül; a kfree/
 * free_pages a free()-t hívja rá (a heap-guard a nem-magic pointert is free()-vel engedi el). */
static inline unsigned long __get_free_pages(gfp_t gfp, unsigned int order)
{
	(void)gfp;
	extern int posix_memalign(void **, unsigned long, unsigned long);
	unsigned long sz = (unsigned long)PAGE_SIZE << order;
	void *p = (void *)0;
	if (posix_memalign(&p, sz, sz) != 0 || !p) return 0;
	__builtin_memset(p, 0, sz);
	return (unsigned long)p;
}
#define __get_free_page(gfp) __get_free_pages((gfp), 0)
static inline void free_pages(unsigned long addr, unsigned int order) { (void)order; kfree((void *)addr); }
#define free_page(addr) free_pages((addr), 0)

#define kmalloc_array(n, s, f) kcalloc((n), (s), (f))
#define kmalloc_node(s, f, node) kmalloc((s), (f))
#define kzalloc_node(s, f, node) kzalloc((s), (f))
#define kmalloc_array_node(n, s, f, node) kcalloc((n), (s), (f))
#define kvmalloc(s, f) kmalloc((s), (f))
#define kvzalloc(s, f) kzalloc((s), (f))
#define kvfree(p) kfree(p)
#define kvmalloc_array(n, s, f) kcalloc((n), (s), (f))
#define kvcalloc(n, s, f) kcalloc((n), (s), (f))
#define kvmalloc_objs(P, COUNT, ...) kcalloc((COUNT), sizeof(typeof(P)), GFP_KERNEL)
#define kvmalloc_obj(P, ...) kmalloc(sizeof(typeof(P)), GFP_KERNEL)

/* kzalloc_obj(P, ...) — a modern kernel objektum-allokátora lecsupaszítva:
 * P lehet típus (struct foo) vagy mutató-deref (*ptr); sizeof(typeof(P)) mindkettőt
 * helyesen kezeli. Az opcionális GFP-argumentumot elnyeljük (mindig GFP_KERNEL). */
#define kzalloc_obj(P, ...)  kzalloc(sizeof(typeof(P)), GFP_KERNEL)
#define kzalloc_objs(P, COUNT, ...) kcalloc((COUNT), sizeof(typeof(P)), GFP_KERNEL)

/* kmalloc_obj(VAR_OR_TYPE, ...) — az argumentum lehet típus (struct foo) vagy
 * meglévő változó; a typeof mindkettőt kezeli. A GFP-t elnyeljük (GFP_KERNEL). */
#define kmalloc_obj(P, ...)  kmalloc(sizeof(typeof(P)), GFP_KERNEL)
#define kmalloc_objs(P, COUNT, ...) kmalloc_array((COUNT), sizeof(typeof(P)), GFP_KERNEL)

/* kmalloc_flex/kzalloc_flex(VAR_OR_TYPE, FAM, COUNT, ...) — rugalmas tömbös
 * objektum: a struct mérete + COUNT elem a FAM tag elemtípusából. */
#define kmalloc_flex(P, FAM, COUNT, ...) \
	kmalloc(sizeof(typeof(P)) + (COUNT) * sizeof((P).FAM[0]), GFP_KERNEL)
#define kzalloc_flex(P, FAM, COUNT, ...) \
	kzalloc(sizeof(typeof(P)) + (COUNT) * sizeof((P).FAM[0]), GFP_KERNEL)

/* kmem_cache — egyszerűsített: fix méretű objektumallokátor malloc fölött. */
struct kmem_cache;
struct kmem_cache_args { unsigned int align; unsigned int useroffset; unsigned int usersize; unsigned int freeptr_offset; bool use_freeptr_offset; unsigned int sheaf_capacity; void (*ctor)(void *); };
struct kmem_cache *__kmem_cache_create_old(const char *name, size_t size, size_t align, unsigned long flags, void (*ctor)(void *));
struct kmem_cache *__kmem_cache_create_args(const char *name, unsigned int object_size, struct kmem_cache_args *args, unsigned int flags);
/* arg-szám diszpécser: 5-arg (régi: name,size,align,flags,ctor) vs 4-arg (új: name,size,&args,flags) */
#define _KMC_PICK(_1,_2,_3,_4,_5,NAME,...) NAME
#define kmem_cache_create(...) _KMC_PICK(__VA_ARGS__, __kmem_cache_create_old, __kmem_cache_create_args)(__VA_ARGS__)
void  kmem_cache_destroy(struct kmem_cache *c);
void *kmem_cache_alloc(struct kmem_cache *c, gfp_t flags);
void *kmem_cache_zalloc(struct kmem_cache *c, gfp_t flags);
void  kmem_cache_free(struct kmem_cache *c, void *obj);
void *kmem_cache_alloc_lru(struct kmem_cache *c, void *lru, gfp_t flags);

#ifndef GFP_NOFS
#define GFP_NOFS     0x11
#endif
#define GFP_HIGHUSER 0x42
#define SLAB_RECLAIM_ACCOUNT   0x00020000UL
#define SLAB_MEM_SPREAD        0x00100000UL
#define SLAB_ACCOUNT           0x04000000UL
#define SLAB_HWCACHE_ALIGN     0x00002000UL
#define SLAB_PANIC             0x00040000UL
#define SLAB_TYPESAFE_BY_RCU   0x00080000UL
#define SLAB_TEMPORARY 0x00000400UL
#define SLAB_NO_MERGE 0x01000000UL
#define KMEM_CACHE(__struct, __flags) \
	kmem_cache_create(#__struct, sizeof(struct __struct), \
		__alignof__(struct __struct), (__flags), NULL)
#define KMEM_CACHE_USERCOPY(__struct, __flags, field) KMEM_CACHE(__struct, __flags)

#endif

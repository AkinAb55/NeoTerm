/* uKernel — slab/vmalloc allokátor a malloc család fölött. */
#include <linux/slab.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <execinfo.h>

/* ===== HEAP-GUARD (UK_HEAP_GUARD env): header(magic+reqsize) + footer-canary; free-kor ellenőrzés.
 * A túlírást FREE-kor kapja el, a túlírt buffer MÉRETÉT + backtrace-t kiírva. Csak debug. ===== */
#define UKG_HMAGIC 0xA110C8EDdeadbeefULL
#define UKG_FMAGIC 0xCAFEBABEfeedfaceULL
struct ukg_hdr { unsigned long long magic; size_t size; struct ukg_hdr *next, *prev; void *abt[8]; int abtn; };
static struct ukg_hdr *ukg_list;        /* élő guard-allokációk láncolt listája */
static int ukg_on(void){ static int v=-1; if(v<0){ const char*e=getenv("UK_HEAP_GUARD"); v=(e&&*e)?1:0; } return v; }
static void ukg_check1(struct ukg_hdr *h){ if(h->magic!=UKG_HMAGIC) return; unsigned long long fm; memcpy(&fm,(char*)(h+1)+h->size,sizeof(fm)); if(fm!=UKG_FMAGIC){ fprintf(stderr,"[GUARD] *** HEAP-TÚLÍRÁS *** buffer size=%zu footer=%#llx\n",h->size,fm); fflush(stderr); fprintf(stderr,"[GUARD] -- a TÚLÍRT buffer ALLOKÁCIÓS backtrace-e:\n"); backtrace_symbols_fd(h->abt,h->abtn,2); fflush(stderr);} }
static void ukg_scan(void){ for(struct ukg_hdr*h=ukg_list;h;h=h->next) ukg_check1(h); }
static void *ukg_alloc(size_t size, int zero)
{
	ukg_scan();                          /* minden allokáció ELŐTT: az összes élő footer ellenőrzése */
	char *b = malloc(sizeof(struct ukg_hdr) + size + sizeof(unsigned long long)); if(!b) return NULL;
	struct ukg_hdr *h = (void*)b; h->magic = UKG_HMAGIC; h->size = size;
	/* A backtrace() drága (stack-unwind) — minden allokációnál hívva annyira lelassít, hogy a
	 * race-ablak eltűnik. Csak a gyanús méretnél (UK_GUARD_BT=<size>) rögzítjük, így a timing
	 * közel marad a guard-mentes futáshoz, és a race megmarad. */
	static long btsz = -2;
	if (btsz == -2) { const char *e = getenv("UK_GUARD_BT"); btsz = e ? atol(e) : -1; }
	h->abtn = (btsz >= 0 && (long)size == btsz) ? backtrace(h->abt, 8) : 0;
	if (h->abtn && getenv("UK_GUARD_BT_LOG")) { fprintf(stderr,"[GUARD] alloc size=%zu @\n",size); backtrace_symbols_fd(h->abt,h->abtn,2); fflush(stderr); }
	char *user = b + sizeof(*h);
	if (zero) memset(user, 0, size);
	unsigned long long fm = UKG_FMAGIC; memcpy(user + size, &fm, sizeof(fm));
	h->next = ukg_list; h->prev = 0; if(ukg_list) ukg_list->prev = h; ukg_list = h;
	return user;
}
static void ukg_check(const void *p)
{
	if(!p) return;
	struct ukg_hdr *h = (struct ukg_hdr*)((char*)p - sizeof(struct ukg_hdr));
	if (h->magic != UKG_HMAGIC) return;   /* nem guard-alloc (idegen ptr) — kihagyjuk */
	unsigned long long fm; memcpy(&fm, (char*)p + h->size, sizeof(fm));
	if (fm != UKG_FMAGIC) {
		fprintf(stderr,"[GUARD] *** HEAP-TÚLÍRÁS *** buffer size=%zu, footer=%#llx\n", h->size, fm); fflush(stderr);
		void *bt[25]; int n=backtrace(bt,25); backtrace_symbols_fd(bt,n,2); fflush(stderr);
	}
}
static void ukg_free(void *p){ if(!p) return; struct ukg_hdr *h=(struct ukg_hdr*)((char*)p-sizeof(*h)); if(h->magic!=UKG_HMAGIC){ free(p); return;} ukg_check(p); ukg_scan();
	if(h->prev) h->prev->next=h->next; else ukg_list=h->next; if(h->next) h->next->prev=h->prev; h->magic=0; free(h); }

void *kmalloc(size_t size, gfp_t flags)
{
	if (ukg_on()) return ukg_alloc(size?size:1, (flags & __GFP_ZERO));
	void *p = malloc(size ? size : 1);
	if (p && (flags & __GFP_ZERO)) memset(p, 0, size);
	return p;
}
void *kzalloc(size_t size, gfp_t flags) { (void)flags; if (ukg_on()) return ukg_alloc(size?size:1,1); return calloc(1, size ? size : 1); }
void *kcalloc(size_t n, size_t size, gfp_t flags) { (void)flags; if (ukg_on()) return ukg_alloc((n?n:1)*(size?size:1),1); return calloc(n ? n : 1, size ? size : 1); }
void *krealloc(void *p, size_t size, gfp_t flags) { (void)flags; if (ukg_on()) { void*np=ukg_alloc(size,1); if(np&&p){ struct ukg_hdr*h=(void*)((char*)p-sizeof(*h)); if(h->magic==UKG_HMAGIC) memcpy(np,p,h->size<size?h->size:size); } if(p) ukg_free(p); return np; } return realloc(p, size); }
void *kmemdup(const void *src, size_t len, gfp_t flags)
{ void *p = kmalloc(len, flags); if (p) memcpy(p, src, len); return p; }
void  kfree(const void *p) { if (ukg_on()) { ukg_free((void*)p); return; } free((void *)p); }

void *vmalloc(size_t size) { if (ukg_on()) return ukg_alloc(size?size:1,0); return malloc(size ? size : 1); }
void *vzalloc(size_t size) { if (ukg_on()) return ukg_alloc(size?size:1,1); return calloc(1, size ? size : 1); }
void  vfree(const void *p) { if (ukg_on()) { ukg_free((void*)p); return; } free((void *)p); }

/* kmem_cache: fix méretű objektumok malloc-kal (poolozás nélkül is helyes). */
struct kmem_cache {
	size_t size;
	void (*ctor)(void *);
	char  name[32];
};

/* a slab.h `kmem_cache_create(...)` makrója átnevezné a definíciónkat (_KMC_PICK) — a NYERS
 * nevet kell exportálni (az ntfs3 azt hivatkozza), ezért itt feloldjuk a makrót. */
#undef kmem_cache_create
struct kmem_cache *kmem_cache_create(const char *name, size_t size, size_t align,
                                     unsigned long flags, void (*ctor)(void *))
{
	(void)align; (void)flags;
	struct kmem_cache *c = malloc(sizeof(*c));
	if (!c) return NULL;
	c->size = size; c->ctor = ctor;
	snprintf(c->name, sizeof(c->name), "%s", name ? name : "cache");
	return c;
}
void kmem_cache_destroy(struct kmem_cache *c) { free(c); }

void *kmem_cache_alloc(struct kmem_cache *c, gfp_t flags)
{
	(void)flags;
	void *o = ukg_on() ? ukg_alloc(c->size,0) : malloc(c->size);
	if (o && c->ctor) c->ctor(o);
	return o;
}
void *kmem_cache_zalloc(struct kmem_cache *c, gfp_t flags)
{ (void)flags; void *o = ukg_on() ? ukg_alloc(c->size,1) : calloc(1, c->size); if (o && c->ctor) c->ctor(o); return o; }
void  kmem_cache_free(struct kmem_cache *c, void *obj) { (void)c; if (ukg_on()) { ukg_free(obj); return; } free(obj); }

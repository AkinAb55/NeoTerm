/* uKernel hamis <linux/idr.h> — minimál ID-allokátor (usb-serial minor-ok).
 * A valódi radix-tree helyett egyszerű dinamikus pointer-tömb. Stub szintű,
 * de a usb-serial.c által hívott API-t hűen adja. */
#ifndef _UK_LINUX_IDR_H
#define _UK_LINUX_IDR_H

#include <linux/types.h>
#include <linux/slab.h>

struct idr {
	void **slots;
	int    size;
};

#define DEFINE_IDR(name) struct idr name = { NULL, 0 }

static inline void idr_init(struct idr *idr) { idr->slots = NULL; idr->size = 0; }
static inline void idr_destroy(struct idr *idr)
{
	kfree(idr->slots);
	idr->slots = NULL;
	idr->size = 0;
}

static inline void *idr_find(const struct idr *idr, unsigned long id)
{
	if ((int)id < 0 || (int)id >= idr->size)
		return NULL;
	return idr->slots[id];
}

/* [start, end) tartományból foglal; visszaad: id vagy negatív hiba. */
static inline int idr_alloc(struct idr *idr, void *ptr, int start, int end, gfp_t gfp)
{
	int i;
	(void)gfp;
	if (start < 0)
		start = 0;
	for (i = start; (end <= 0 || i < end); i++) {
		if (i >= idr->size) {
			int newsize = i + 16;
			void **n = krealloc(idr->slots, newsize * sizeof(void *), 0);
			if (!n)
				return -12 /*ENOMEM*/;
			for (int j = idr->size; j < newsize; j++)
				n[j] = NULL;
			idr->slots = n;
			idr->size = newsize;
		}
		if (!idr->slots[i]) {
			idr->slots[i] = ptr;
			return i;
		}
	}
	return -28 /*ENOSPC*/;
}

static inline void idr_remove(struct idr *idr, unsigned long id)
{
	if ((int)id >= 0 && (int)id < idr->size)
		idr->slots[id] = NULL;
}

#endif

/* uKernel shim — minimális kfifo (a usb_serial_port write_fifo-jához).
 * A valódi kernel kfifo bonyolult makró-gépezet; itt egy egyszerű bájt-FIFO,
 * ami a usb-serial által hívott függvényeket (alloc/free/len/in/out/reset) adja. */
#ifndef _UKNL_KFIFO_H
#define _UKNL_KFIFO_H
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/string.h>

struct kfifo {
	unsigned char *data;
	unsigned int size;   /* kapacitás (2 hatványa) */
	unsigned int in;     /* írási pozíció (monoton) */
	unsigned int out;    /* olvasási pozíció (monoton) */
};

static inline int kfifo_alloc(struct kfifo *f, unsigned int size, gfp_t gfp)
{
	(void)gfp;
	unsigned int s = 1; while (s < size) s <<= 1;
	f->data = kmalloc(s, 0); if (!f->data) return -12 /*ENOMEM*/;
	f->size = s; f->in = f->out = 0; return 0;
}
static inline void kfifo_free(struct kfifo *f) { kfree(f->data); f->data = NULL; f->size = 0; f->in = f->out = 0; }
static inline void kfifo_reset(struct kfifo *f) { f->in = f->out = 0; }
/* kfifo_reset_out — csak az olvasó oldalt ürítjük (out = in); a usb-serial
 * generic.c a write_fifo lezárásakor hívja. */
static inline void kfifo_reset_out(struct kfifo *f) { f->out = f->in; }
static inline unsigned int kfifo_len(struct kfifo *f) { return f->in - f->out; }
static inline unsigned int kfifo_size(struct kfifo *f) { return f->size; }
static inline unsigned int kfifo_avail(struct kfifo *f) { return f->size - (f->in - f->out); }
static inline int kfifo_is_empty(struct kfifo *f) { return f->in == f->out; }
static inline int kfifo_is_full(struct kfifo *f) { return (f->in - f->out) >= f->size; }

static inline unsigned int kfifo_in(struct kfifo *f, const void *buf, unsigned int len)
{
	unsigned int avail = f->size - (f->in - f->out); if (len > avail) len = avail;
	for (unsigned int i = 0; i < len; i++) f->data[(f->in + i) & (f->size - 1)] = ((const unsigned char *)buf)[i];
	f->in += len; return len;
}
static inline unsigned int kfifo_out(struct kfifo *f, void *buf, unsigned int len)
{
	unsigned int have = f->in - f->out; if (len > have) len = have;
	for (unsigned int i = 0; i < len; i++) ((unsigned char *)buf)[i] = f->data[(f->out + i) & (f->size - 1)];
	f->out += len; return len;
}
/* kfifo_out_locked / kfifo_in_locked: a usb-serial spinlockkal hívja — a lockot a
 * hívó adja, mi csak a FIFO-műveletet végezzük (a shim spinlockja amúgy is no-op). */
#define kfifo_in_locked(f, buf, len, lock)  ({ (void)(lock); kfifo_in((f), (buf), (len)); })
#define kfifo_out_locked(f, buf, len, lock) ({ (void)(lock); kfifo_out((f), (buf), (len)); })
#define kfifo_len_locked(f, lock)           ({ (void)(lock); kfifo_len((f)); })

#endif

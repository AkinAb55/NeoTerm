/* uKernel hamis <linux/uio.h> — iov_iter (felhasználói I/O iterátor) stub. */
#ifndef _UK_LINUX_UIO_H
#define _UK_LINUX_UIO_H
#include <linux/types.h>

enum iter_type { ITER_UBUF, ITER_IOVEC, ITER_KVEC, ITER_BVEC };

struct iovec { void *iov_base; size_t iov_len; };
struct kvec { void *iov_base; size_t iov_len; };

struct iov_iter {
	u8 iter_type;
	bool data_source;     /* true=READ a forrásból, kompatibilitás */
	size_t iov_count;
	size_t count;
	unsigned long nr_segs;
	loff_t xarray_start;
	const void *uk_data;  /* uKernel: az írandó/olvasandó adatpuffer (a fake iter-hez) */
};

static inline size_t iov_iter_count(const struct iov_iter *i) { return i->count; }
static inline int iov_iter_rw(const struct iov_iter *i) { return i->data_source; }
static inline bool iter_is_iovec(const struct iov_iter *i) { return i->iter_type == ITER_IOVEC; }
size_t copy_to_iter(const void *addr, size_t bytes, struct iov_iter *i);
size_t copy_from_iter(void *addr, size_t bytes, struct iov_iter *i);
#define READ 0
#define WRITE 1
#endif

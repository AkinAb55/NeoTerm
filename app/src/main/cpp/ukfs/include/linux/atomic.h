/* uKernel hamis <linux/atomic.h> — GCC __atomic builtinek fölött. */
#ifndef _UK_LINUX_ATOMIC_H
#define _UK_LINUX_ATOMIC_H

#include <linux/types.h>

#define ATOMIC_INIT(i) { (i) }

static inline int  atomic_read(const atomic_t *v) { return __atomic_load_n(&v->counter, __ATOMIC_SEQ_CST); }
static inline void atomic_set(atomic_t *v, int i)  { __atomic_store_n(&v->counter, i, __ATOMIC_SEQ_CST); }
static inline int  atomic_add_return(int i, atomic_t *v) { return __atomic_add_fetch(&v->counter, i, __ATOMIC_SEQ_CST); }
static inline int  atomic_sub_return(int i, atomic_t *v) { return __atomic_sub_fetch(&v->counter, i, __ATOMIC_SEQ_CST); }
static inline void atomic_inc(atomic_t *v) { (void)atomic_add_return(1, v); }
static inline void atomic_dec(atomic_t *v) { (void)atomic_sub_return(1, v); }
static inline int  atomic_inc_return(atomic_t *v) { return atomic_add_return(1, v); }
static inline int  atomic_dec_return(atomic_t *v) { return atomic_sub_return(1, v); }
static inline int  atomic_dec_and_test(atomic_t *v) { return atomic_sub_return(1, v) == 0; }
static inline int  atomic_inc_and_test(atomic_t *v) { return atomic_add_return(1, v) == 0; }
static inline void atomic_add(int i, atomic_t *v) { (void)atomic_add_return(i, v); }
static inline void atomic_sub(int i, atomic_t *v) { (void)atomic_sub_return(i, v); }
static inline int  atomic_cmpxchg(atomic_t *v, int old, int new)
{ __atomic_compare_exchange_n(&v->counter, &old, new, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST); return old; }
static inline int  atomic_xchg(atomic_t *v, int new) { return __atomic_exchange_n(&v->counter, new, __ATOMIC_SEQ_CST); }
static inline int  atomic_add_unless(atomic_t *v, int a, int u){ int c=atomic_read(v); while(c!=u){ if(__atomic_compare_exchange_n(&v->counter,&c,c+a,0,__ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST)) return 1; } return 0; }
static inline int  atomic_inc_not_zero(atomic_t *v){ return atomic_add_unless(v,1,0); }

/* set_bit / test_bit egyszerű long-tömbökön */
static inline void set_bit(long nr, volatile unsigned long *addr)
{ __atomic_fetch_or(&addr[nr / BITS_PER_LONG], 1UL << (nr % BITS_PER_LONG), __ATOMIC_SEQ_CST); }
static inline void clear_bit(long nr, volatile unsigned long *addr)
{ __atomic_fetch_and(&addr[nr / BITS_PER_LONG], ~(1UL << (nr % BITS_PER_LONG)), __ATOMIC_SEQ_CST); }
static inline int test_bit(long nr, const volatile unsigned long *addr)
{ return (addr[nr / BITS_PER_LONG] >> (nr % BITS_PER_LONG)) & 1UL; }
static inline int test_and_set_bit(long nr, volatile unsigned long *addr)
{ unsigned long m = 1UL << (nr % BITS_PER_LONG);
  return (__atomic_fetch_or(&addr[nr / BITS_PER_LONG], m, __ATOMIC_SEQ_CST) & m) != 0; }
static inline int test_and_clear_bit(long nr, volatile unsigned long *addr)
{ unsigned long m = 1UL << (nr % BITS_PER_LONG);
  return (__atomic_fetch_and(&addr[nr / BITS_PER_LONG], ~m, __ATOMIC_SEQ_CST) & m) != 0; }

/* nem-atomi bit-műveletek (__ prefix) — userspace-ben elég a sima írás */
static inline void __set_bit(long nr, volatile unsigned long *addr)
{ addr[nr / BITS_PER_LONG] |= 1UL << (nr % BITS_PER_LONG); }
static inline void __clear_bit(long nr, volatile unsigned long *addr)
{ addr[nr / BITS_PER_LONG] &= ~(1UL << (nr % BITS_PER_LONG)); }
static inline void __change_bit(long nr, volatile unsigned long *addr)
{ addr[nr / BITS_PER_LONG] ^= 1UL << (nr % BITS_PER_LONG); }
static inline int __test_and_set_bit(long nr, volatile unsigned long *addr)
{ unsigned long m = 1UL << (nr % BITS_PER_LONG); int o = (addr[nr/BITS_PER_LONG]&m)!=0;
  addr[nr/BITS_PER_LONG] |= m; return o; }
static inline int __test_and_clear_bit(long nr, volatile unsigned long *addr)
{ unsigned long m = 1UL << (nr % BITS_PER_LONG); int o = (addr[nr/BITS_PER_LONG]&m)!=0;
  addr[nr/BITS_PER_LONG] &= ~m; return o; }

/* lock-os bit-műveletek (usb-serial generic.c) — userspace-ben a sima
 * megfelelőjük; a barrier-ek a __atomic SEQ_CST miatt no-opok. */
static inline int test_and_set_bit_lock(long nr, volatile unsigned long *addr)
{ return test_and_set_bit(nr, addr); }
static inline void clear_bit_unlock(long nr, volatile unsigned long *addr)
{ clear_bit(nr, addr); }
#define smp_mb__before_atomic() __atomic_thread_fence(__ATOMIC_SEQ_CST)
#define smp_mb__after_atomic()  __atomic_thread_fence(__ATOMIC_SEQ_CST)

/* find_first_bit — az első beállított bit indexe [0, size) tartományban. */
static inline unsigned long find_first_bit(const unsigned long *addr, unsigned long size)
{
	unsigned long i;
	for (i = 0; i < size; i++)
		if (addr[i / BITS_PER_LONG] & (1UL << (i % BITS_PER_LONG)))
			return i;
	return size;
}

#endif

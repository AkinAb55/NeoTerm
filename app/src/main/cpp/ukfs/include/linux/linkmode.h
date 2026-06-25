#ifndef _UK_LINUX_LINKMODE_H
#define _UK_LINUX_LINKMODE_H
#include <linux/bitops.h>
#include <linux/ethtool.h>
static inline void linkmode_zero(unsigned long *dst) { bitmap_zero(dst, __ETHTOOL_LINK_MODE_MASK_NBITS); }
static inline void linkmode_set_bit(int nr, unsigned long *addr) { __set_bit(nr, addr); }
static inline void linkmode_clear_bit(int nr, unsigned long *addr) { __clear_bit(nr, addr); }
static inline int  linkmode_test_bit(int nr, const unsigned long *addr) { return test_bit(nr, addr); }
static inline void linkmode_copy(unsigned long *dst, const unsigned long *src) { bitmap_copy(dst, src, __ETHTOOL_LINK_MODE_MASK_NBITS); }
static inline void linkmode_mod_bit(int nr, unsigned long *addr, int set) { if (set) __set_bit(nr, addr); else __clear_bit(nr, addr); }
static inline void mii_eee_cap1_mod_linkmode_t(unsigned long *adv, u32 val) { (void)adv;(void)val; }
static inline u32 linkmode_to_mii_eee_cap1_t(unsigned long *adv) { (void)adv; return 0; }
#endif

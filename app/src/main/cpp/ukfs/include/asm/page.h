/* uKernel hamis <asm/page.h>. */
#ifndef _UK_ASM_PAGE_H
#define _UK_ASM_PAGE_H
#ifndef PAGE_SHIFT
#define PAGE_SHIFT 12
#endif
#ifndef PAGE_SIZE
#define PAGE_SIZE (1UL << PAGE_SHIFT)
#endif
#define PAGE_MASK (~(PAGE_SIZE - 1))
#define PAGE_ALIGN(addr) (((addr) + PAGE_SIZE - 1) & PAGE_MASK)
#endif

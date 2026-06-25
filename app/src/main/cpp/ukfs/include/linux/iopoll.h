#ifndef _UK_LINUX_IOPOLL_H
#define _UK_LINUX_IOPOLL_H
#include <linux/types.h>
#include <linux/errno.h>
/* a 'op' kifejezes deklaralhat valtozot is (u16 x = ...); egyszer ertekeljuk
 * ki, majd a 'cond'-ot. Userspace-ben nem pollozunk valodi idoig. */
#define read_poll_timeout(op, val, cond, sleep_us, timeout_us, sleep_before, args...) \
({ \
	int __ret = 0; \
	(val) = (op)(args); \
	if (!(cond)) __ret = -ETIMEDOUT; \
	__ret; \
})
#define readx_poll_timeout(op, addr, val, cond, sleep_us, timeout_us) \
	read_poll_timeout(op, val, cond, sleep_us, timeout_us, false, addr)
#define poll_timeout_us(op, cond, sleep_us, timeout_us, sleep_before) \
({ \
	int __ret = 0; \
	op; \
	if (!(cond)) __ret = -ETIMEDOUT; \
	__ret; \
})
#endif

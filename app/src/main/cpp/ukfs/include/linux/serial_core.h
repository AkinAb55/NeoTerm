#ifndef _UK_LINUX_SERIAL_CORE_H
#define _UK_LINUX_SERIAL_CORE_H
#include <linux/types.h>
#include <linux/serial.h>
struct uart_icount {
	__u32 cts, dsr, rng, dcd, rx, tx, frame, overrun, parity, brk, buf_overrun;
};
#endif

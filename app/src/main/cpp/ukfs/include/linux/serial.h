/* uKernel hamis <linux/serial.h> — async_icount + serial_struct,
 * csak a usb-serial mag által hivatkozott mezőkkel. */
#ifndef _UK_LINUX_SERIAL_H
#define _UK_LINUX_SERIAL_H

#include <linux/types.h>

/* megszakítás-számlálók (generic.c / ch341.c port->icount) */
struct async_icount {
	__u32 cts, dsr, rng, dcd, tx, rx;
	__u32 frame, parity, overrun, brk;
	__u32 buf_overrun;
};

/* TIOCGSERIAL/TIOCSSERIAL adat (usb-serial.c serial_get/set_serial) */
struct serial_struct {
	int	type;
	int	line;
	unsigned int port;
	int	irq;
	int	flags;
	int	xmit_fifo_size;
	int	custom_divisor;
	int	baud_base;
	unsigned short close_delay;
	unsigned short closing_wait;
};

/* ASYNC_* flag-ek (serial_struct.flags) */
#define ASYNCB_SPD_HI		4
#define ASYNCB_SPD_VHI		5
#define ASYNCB_LOW_LATENCY	13
#define ASYNCB_SPD_SHI		12
#define ASYNC_SPD_HI		(1U << ASYNCB_SPD_HI)
#define ASYNC_SPD_VHI		(1U << ASYNCB_SPD_VHI)
#define ASYNC_SPD_SHI		(1U << ASYNCB_SPD_SHI)
#define ASYNC_SPD_CUST		(ASYNC_SPD_HI | ASYNC_SPD_VHI)
#define ASYNC_SPD_MASK		(ASYNC_SPD_HI | ASYNC_SPD_VHI | ASYNC_SPD_SHI)
#define ASYNC_LOW_LATENCY	(1U << ASYNCB_LOW_LATENCY)
#define ASYNC_FLAGS		0x0FFF
#define ASYNC_USR_MASK		(ASYNC_SPD_MASK | ASYNC_LOW_LATENCY)

/* RS485 (xr_serial.c, mos7720.c, …) */
struct serial_rs485 {
	__u32	flags;
	__u32	delay_rts_before_send;
	__u32	delay_rts_after_send;
	union {
		__u32	padding[5];
		struct {
			__u8	addr_recv;
			__u8	addr_dest;
			__u8	padding0[2];
			__u32	padding1[4];
		};
	};
};

#define SER_RS485_ENABLED		(1 << 0)
#define SER_RS485_RTS_ON_SEND		(1 << 1)
#define SER_RS485_RTS_AFTER_SEND	(1 << 2)
#define SER_RS485_RX_DURING_TX		(1 << 4)
#define SER_RS485_TERMINATE_BUS		(1 << 5)
#define SER_RS485_ADDRB			(1 << 6)
#define SER_RS485_ADDR_RECV		(1 << 7)
#define SER_RS485_ADDR_DEST		(1 << 8)

/* generic.c usb_serial_generic_get_icount() ezt tölti ki */
struct serial_icounter_struct {
	int cts, dsr, rng, dcd, rx, tx;
	int frame, overrun, parity, brk;
	int buf_overrun;
};

#define ASYNC_CLOSING_WAIT_NONE		0xffff
#define ASYNC_CLOSING_WAIT_INF		0

/* Helper for dealing with UART_LCR_WLEN* defines (a rendszer <linux/serial_reg.h>
 * csak az UART_LCR_WLEN5..8 konstansokat tartalmazza, a függvény-szerű makrót nem) */
#ifndef UART_LCR_WLEN
#define UART_LCR_WLEN(x)	((x) - 5)
#endif

#endif

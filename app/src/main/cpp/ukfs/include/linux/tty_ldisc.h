/* uKernel hamis <linux/tty_ldisc.h> — line discipline ref (stub).
 * Csak generic.c usb_serial_handle_dcd_change() használja. */
#ifndef _UK_LINUX_TTY_LDISC_H
#define _UK_LINUX_TTY_LDISC_H

struct tty_struct;

struct tty_ldisc_ops {
	void (*dcd_change)(struct tty_struct *tty, unsigned int status);
};

struct tty_ldisc {
	struct tty_ldisc_ops *ops;
};

struct tty_ldisc *tty_ldisc_ref(struct tty_struct *tty);
void tty_ldisc_deref(struct tty_ldisc *ld);

#endif

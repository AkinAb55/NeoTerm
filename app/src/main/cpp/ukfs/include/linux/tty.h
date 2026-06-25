/* uKernel hamis <linux/tty.h> — minimál tty-réteg a usb-serial maghoz.
 * Csak a 4 forrás (usb-serial.c, generic.c, bus.c, ch341.c) által ténylegesen
 * használt struktúrák/makrók/prototípusok. A viselkedés stub (shim/tty/tty.c). */
#ifndef _UK_LINUX_TTY_H
#define _UK_LINUX_TTY_H

#include <linux/types.h>
#include <linux/wait.h>
#include <linux/mutex.h>
#include <linux/tty_port.h>
#include <linux/tty_ldisc.h>

/* modem control bitek (TIOCM*) — ch341.c / generic.c */
#define TIOCM_LE	0x001
#define TIOCM_DTR	0x002
#define TIOCM_RTS	0x004
#define TIOCM_ST	0x008
#define TIOCM_SR	0x010
#define TIOCM_CTS	0x020
#define TIOCM_CD	0x040
#define TIOCM_RI	0x080
#define TIOCM_DSR	0x100
#define TIOCM_OUT1	0x2000
#define TIOCM_OUT2	0x4000
#define TIOCM_LOOP	0x8000
#define TIOCM_CAR	TIOCM_CD
#define TIOCM_RNG	TIOCM_RI

/* ioctl-ek (usb-serial.c serial_ioctl) */
#define TIOCMIWAIT	0x545C
#define TIOCGSERIAL	0x541E
#define TIOCSSERIAL	0x541F
#define TIOCSERGETLSR	0x5459
#define TIOCSER_TEMT	0x01
#define TIOCGRS485	0x542E
#define TIOCSRS485	0x542F
#define TCFLSH		0x540B

/* ===== termios bitek (termbits) ===== */
typedef unsigned char  cc_t;
typedef unsigned int   speed_t;
typedef unsigned int   tcflag_t;

#define NCCS 19
struct ktermios {
	tcflag_t c_iflag;
	tcflag_t c_oflag;
	tcflag_t c_cflag;
	tcflag_t c_lflag;
	cc_t     c_line;
	cc_t     c_cc[NCCS];
	speed_t  c_ispeed;
	speed_t  c_ospeed;
};

/* c_cflag bitek */
#define CBAUD	0x0000100f
#define B0	0x00000000
#define B9600	0x0000000d
#define CSIZE	0x00000030
#define CS5	0x00000000
#define CS6	0x00000010
#define CS7	0x00000020
#define CS8	0x00000030
#define CSTOPB	0x00000040
#define CREAD	0x00000080
#define PARENB	0x00000100
#define PARODD	0x00000200
#define HUPCL	0x00000400
#define CLOCAL	0x00000800
#define CRTSCTS	0x80000000
#define CMSPAR	0x40000000
#define CBAUDEX	0x00001000

/* c_iflag bitek */
#define IGNBRK	0x0001
#define BRKINT	0x0002
#define IGNPAR	0x0004
#define PARMRK	0x0008
#define INPCK	0x0010
#define ISTRIP	0x0020
#define INLCR	0x0040
#define IGNCR	0x0080
#define ICRNL	0x0100
#define IUCLC	0x0200
#define IXON	0x0400
#define IXANY	0x0800
#define IXOFF	0x1000
#define IMAXBEL	0x2000
#define IUTF8	0x4000

/* c_oflag bitek */
#define OPOST	0x0001
#define OLCUC	0x0002
#define ONLCR	0x0004
#define OCRNL	0x0008
#define ONOCR	0x0010
#define ONLRET	0x0020

/* c_lflag bitek */
#define ISIG	0x0001
#define ICANON	0x0002
#define ECHO	0x0008
#define ECHOE	0x0010
#define ECHOK	0x0020
#define ECHONL	0x0040
#define NOFLSH	0x0080
#define TOSTOP	0x0100
#define IEXTEN	0x8000

/* c_cc indexek */
#define VINTR	0
#define VQUIT	1
#define VERASE	2
#define VKILL	3
#define VEOF	4
#define VTIME	5
#define VMIN	6
#define VSWTC	7
#define VSTART	8
#define VSTOP	9
#define VSUSP	10
#define VEOL	11
#define VREPRINT 12
#define VDISCARD 13
#define VWERASE	14
#define VLNEXT	15
#define VEOL2	16

/* karakter-alapértékek */
#define START_CHAR(tty)	((tty)->termios.c_cc[VSTART])
#define STOP_CHAR(tty)	((tty)->termios.c_cc[VSTOP])

/* line discipline szám */
#define N_TTY	0

/* flip-buffer flag-ek (generic.c / driverek) */
#define TTY_BREAK	2
#define TTY_FRAME	3
#define TTY_PARITY	4
#define TTY_OVERRUN	5

/* további baud-konstansok néhány driverhez */
#define B50	0x00000001
#define B75	0x00000002
#define B110	0x00000003
#define B134	0x00000004
#define B150	0x00000005
#define B200	0x00000006
#define B300	0x00000007
#define B600	0x00000008
#define B1200	0x00000009
#define B1800	0x0000000a
#define B2400	0x0000000b
#define B4800	0x0000000c
#define B19200	0x0000000e
#define B38400	0x0000000f

/* ===== tty_struct ===== */
struct tty_struct {
	int		index;
	struct ktermios	termios;
	void		*driver_data;
	struct tty_driver *driver;
	struct tty_port	*port;
	struct rw_semaphore termios_rwsem;
	unsigned long	flags;
};

/* tty_struct.flags bitek */
#define TTY_THROTTLED		0
#define TTY_IO_ERROR		1
#define TTY_OTHER_CLOSED	2
#define TTY_EXCLUSIVE		3
#define TTY_DO_WRITE_WAKEUP	5
#define TTY_LDISC_OPEN		11
#define TTY_HUPPED		18
#define TTY_HUPPING		19
#define TTY_NO_WRITE_SPLIT	17

/* C_* segéd-makrók a tty->termios.c_cflag fölött */
#define C_BAUD(tty)	((tty)->termios.c_cflag & CBAUD)
#define C_CSIZE(tty)	((tty)->termios.c_cflag & CSIZE)
#define C_CSTOPB(tty)	(!!((tty)->termios.c_cflag & CSTOPB))
#define C_PARENB(tty)	(!!((tty)->termios.c_cflag & PARENB))
#define C_PARODD(tty)	(!!((tty)->termios.c_cflag & PARODD))
#define C_CMSPAR(tty)	(!!((tty)->termios.c_cflag & CMSPAR))
#define C_CRTSCTS(tty)	(!!((tty)->termios.c_cflag & CRTSCTS))
#define C_CLOCAL(tty)	(!!((tty)->termios.c_cflag & CLOCAL))
#define C_CREAD(tty)	(!!((tty)->termios.c_cflag & CREAD))
#define C_HUPCL(tty)	(!!((tty)->termios.c_cflag & HUPCL))

/* I_* / O_* / L_* segéd-makrók */
#define I_IXON(tty)	(!!((tty)->termios.c_iflag & IXON))
#define I_IXOFF(tty)	(!!((tty)->termios.c_iflag & IXOFF))
#define I_IXANY(tty)	(!!((tty)->termios.c_iflag & IXANY))
#define I_INPCK(tty)	(!!((tty)->termios.c_iflag & INPCK))
#define I_ISTRIP(tty)	(!!((tty)->termios.c_iflag & ISTRIP))
#define I_IGNBRK(tty)	(!!((tty)->termios.c_iflag & IGNBRK))
#define I_BRKINT(tty)	(!!((tty)->termios.c_iflag & BRKINT))
#define I_IGNPAR(tty)	(!!((tty)->termios.c_iflag & IGNPAR))
#define I_PARMRK(tty)	(!!((tty)->termios.c_iflag & PARMRK))
#define O_OPOST(tty)	(!!((tty)->termios.c_oflag & OPOST))
#define L_ICANON(tty)	(!!((tty)->termios.c_lflag & ICANON))
#define L_ISIG(tty)	(!!((tty)->termios.c_lflag & ISIG))
#define L_IEXTEN(tty)	(!!((tty)->termios.c_lflag & IEXTEN))
#define L_ECHO(tty)	(!!((tty)->termios.c_lflag & ECHO))

/* TTY_NORMAL flag a flip-bufferhez (generic.c) */
#define TTY_NORMAL	0

extern struct ktermios tty_std_termios;

/* termios/baud segédfüggvények (stub) */
speed_t tty_termios_baud_rate(const struct ktermios *termios);
speed_t tty_get_baud_rate(struct tty_struct *tty);
int  tty_termios_hw_change(const struct ktermios *a, const struct ktermios *b);
void tty_termios_copy_hw(struct ktermios *new, const struct ktermios *old);
void tty_termios_encode_baud_rate(struct ktermios *termios,
				  speed_t ibaud, speed_t obaud);
void tty_encode_baud_rate(struct tty_struct *tty,
			  speed_t ibaud, speed_t obaud);

int  tty_throttled(struct tty_struct *tty);
int  tty_put_char(struct tty_struct *tty, unsigned char ch);
void tty_hangup(struct tty_struct *tty);
void tty_kref_put(struct tty_struct *tty);
void tty_wakeup(struct tty_struct *tty);

#endif

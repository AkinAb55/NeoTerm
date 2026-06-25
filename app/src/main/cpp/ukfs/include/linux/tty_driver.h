/* uKernel hamis <linux/tty_driver.h> — tty_driver + tty_operations (stub). */
#ifndef _UK_LINUX_TTY_DRIVER_H
#define _UK_LINUX_TTY_DRIVER_H

#include <linux/types.h>
#include <linux/tty.h>

struct tty_struct;
struct file;
struct serial_struct;
struct serial_icounter_struct;
struct seq_file;

struct tty_operations {
	int  (*install)(struct tty_driver *driver, struct tty_struct *tty);
	int  (*open)(struct tty_struct *tty, struct file *filp);
	void (*close)(struct tty_struct *tty, struct file *filp);
	void (*cleanup)(struct tty_struct *tty);
	void (*hangup)(struct tty_struct *tty);
	ssize_t (*write)(struct tty_struct *tty, const u8 *buf, size_t count);
	unsigned int (*write_room)(struct tty_struct *tty);
	unsigned int (*chars_in_buffer)(struct tty_struct *tty);
	int  (*ioctl)(struct tty_struct *tty, unsigned int cmd, unsigned long arg);
	void (*set_termios)(struct tty_struct *tty, const struct ktermios *old);
	void (*throttle)(struct tty_struct *tty);
	void (*unthrottle)(struct tty_struct *tty);
	void (*flush_buffer)(struct tty_struct *tty);
	void (*flush_chars)(struct tty_struct *tty);
	void (*send_xchar)(struct tty_struct *tty, u8 ch);
	int  (*put_char)(struct tty_struct *tty, u8 ch);
	void (*wait_until_sent)(struct tty_struct *tty, int timeout);
	int  (*break_ctl)(struct tty_struct *tty, int state);
	int  (*tiocmget)(struct tty_struct *tty);
	int  (*tiocmset)(struct tty_struct *tty, unsigned int set, unsigned int clear);
	int  (*get_icount)(struct tty_struct *tty, struct serial_icounter_struct *icount);
	int  (*get_serial)(struct tty_struct *tty, struct serial_struct *p);
	int  (*set_serial)(struct tty_struct *tty, struct serial_struct *p);
	int  (*proc_show)(struct seq_file *m, void *driver);
};

struct tty_driver {
	const char	*driver_name;
	const char	*name;
	int		major;
	int		minor_start;
	short		type;
	short		subtype;
	unsigned long	flags;
	struct ktermios	init_termios;
	struct ktermios	**termios;
	const struct tty_operations *ops;
	struct module	*owner;
};

/* tty_alloc_driver flags */
#define TTY_DRIVER_REAL_RAW		0x0004
#define TTY_DRIVER_DYNAMIC_DEV		0x0008

/* tty driver type / subtype */
#define TTY_DRIVER_TYPE_SERIAL		0x0003
#define SERIAL_TYPE_NORMAL		1

struct tty_driver *tty_alloc_driver(unsigned int lines, unsigned long flags);
int  tty_register_driver(struct tty_driver *driver);
void tty_unregister_driver(struct tty_driver *driver);
void tty_set_operations(struct tty_driver *driver, const struct tty_operations *op);
void tty_driver_kref_put(struct tty_driver *driver);
int  tty_standard_install(struct tty_driver *driver, struct tty_struct *tty);
void tty_unregister_device(struct tty_driver *driver, unsigned index);

#endif

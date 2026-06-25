/* uKernel hamis <linux/tty_port.h> — a usb_serial_port beágyazott tty_port-ja. */
#ifndef _UK_LINUX_TTY_PORT_H
#define _UK_LINUX_TTY_PORT_H

#include <linux/types.h>
#include <linux/wait.h>
#include <linux/mutex.h>
#include <linux/kref.h>

struct tty_struct;
struct tty_port;
struct file;
struct device;
struct tty_driver;

struct tty_port_operations {
	bool (*carrier_raised)(struct tty_port *port);
	void (*dtr_rts)(struct tty_port *port, bool active);
	int  (*activate)(struct tty_port *port, struct tty_struct *tty);
	void (*shutdown)(struct tty_port *port);
	void (*destruct)(struct tty_port *port);
};

struct tty_port {
	const struct tty_port_operations *ops;
	struct mutex		mutex;
	struct tty_struct	*tty;
	unsigned long		close_delay;
	unsigned long		closing_wait;
	int			drain_delay;
	wait_queue_head_t	open_wait;
	wait_queue_head_t	delta_msr_wait;
	int			console;	/* port-on console fut-e */
	unsigned long		flags;
	int			count;		/* nyitas-szamlalo */
	int			blocked_open;
	struct kref		kref;
};

void tty_port_init(struct tty_port *port);
void tty_port_destroy(struct tty_port *port);
int  tty_port_open(struct tty_port *port, struct tty_struct *tty, struct file *filp);
int  tty_port_close(struct tty_port *port, struct tty_struct *tty, struct file *filp);
void tty_port_hangup(struct tty_port *port);
struct tty_struct *tty_port_tty_get(struct tty_port *port);
void tty_port_tty_wakeup(struct tty_port *port);
void tty_port_tty_hangup(struct tty_port *port, bool check_clocal);
void tty_port_tty_vhangup(struct tty_port *port);
int  tty_port_initialized(struct tty_port *port);
struct tty_port *tty_port_get(struct tty_port *port);
void tty_port_put(struct tty_port *port);
void tty_port_tty_set(struct tty_port *port, struct tty_struct *tty);
struct device *tty_port_register_device(struct tty_port *port,
		struct tty_driver *driver, unsigned index, struct device *device);
struct device *tty_port_register_device_attr(struct tty_port *port,
		struct tty_driver *driver, unsigned index, struct device *device,
		void *drvdata, const void *attr_grp);

#endif

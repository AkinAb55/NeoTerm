/* uKernel hamis <linux/tty_flip.h> — flip-buffer beszúrás (stub). */
#ifndef _UK_LINUX_TTY_FLIP_H
#define _UK_LINUX_TTY_FLIP_H

#include <linux/types.h>
#include <linux/tty_port.h>

int  tty_insert_flip_char(struct tty_port *port, unsigned char ch, char flag);
int  tty_insert_flip_string(struct tty_port *port, const unsigned char *chars,
			    size_t size);
int  tty_insert_flip_string_fixed_flag(struct tty_port *port,
		const unsigned char *chars, char flag, size_t size);
int  tty_buffer_request_room(struct tty_port *port, size_t size);
void tty_flip_buffer_push(struct tty_port *port);

#endif

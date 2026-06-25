/* uKernel — a tty-shim runtime publikus API-ja (a teszt-harness / LD_PRELOAD-bridge számára).
 * A valódi Linux tty-driver (cdc-acm/usb-serial) a flip-bufferbe teszi a vett adatot
 * (tty_insert_flip_string + tty_flip_buffer_push); az alábbi uk_tty_* a bridge oldaláról
 * olvassa azt, ill. vezérli a port-életciklust (open→activate, close→shutdown). */
#ifndef _UK_UKTTY_H
#define _UK_UKTTY_H
#include <stddef.h>

struct tty_port;
struct tty_driver;

/* a flip-bufferbe gyűlt (a driver által vett) adat kiolvasása; visszaad bájtszámot (0 ha üres). */
long uk_tty_flip_read(struct tty_port *port, void *buf, size_t len);
/* mennyi olvasható adat van a flip-bufferben (poll-hoz) */
long uk_tty_flip_avail(struct tty_port *port);

/* a regisztrált tty_driver-ek lekérdezése a bridge-hez (név szerint, pl. "ttyACM") */
struct tty_driver *uk_tty_driver_by_name(const char *name);
/* egy regisztrált driver index-edik tty_port-ja (a register_device-ból), vagy NULL */
struct tty_port *uk_tty_port_by_index(struct tty_driver *driver, unsigned index);

/* ===== magas szintű handle-API (a LD_PRELOAD-bridge ezt használja — DRIVER-FÜGGETLEN) =====
 * A bridge nem nyúl kernel-structhoz (tty_struct/operations): csak ezen az opaque handle-ön át
 * nyit/ír/olvas/zár. A megnyitás elvégzi az install + open (tty_port_open -> activate) lépéseket,
 * az írás a driver ops->write-ját hívja (-> bulk OUT), az olvasás a flip-bufferből húz. */
void *uk_tty_open(const char *driver_name, unsigned index);   /* NULL ha nincs ilyen port */
long  uk_tty_write(void *handle, const void *buf, size_t len);
long  uk_tty_read(void *handle, void *buf, size_t len);       /* nem-blokkoló: 0 ha üres */
long  uk_tty_avail(void *handle);
void  uk_tty_close(void *handle);

/* ===== soros-beállítások (line-coding + modem-control) — DRIVER-FÜGGETLEN =====
 * A bridge a POSIX termios/ioctl-okból ide fordítja a beállításokat; a runtime felépíti a kernel
 * ktermios-t és meghívja a driver ops->set_termios-át (cdc-acm: SET_LINE_CODING control transfer),
 * ill. az ops->tiocmget/tiocmset-et (cdc-acm: SET_CONTROL_LINE_STATE -> DTR/RTS). */
int uk_tty_set_line(void *handle, unsigned int baud, int bits, int parity, int stop); /* parity:0=N,1=O,2=E */
int uk_tty_get_line(void *handle, unsigned int *baud, int *bits, int *parity, int *stop);
int uk_tty_tiocmget(void *handle);                       /* TIOCM_* bitmaszk, vagy <0 hiba */
int uk_tty_tiocmset(void *handle, int set, int clear);   /* TIOCM_* bitek be/ki */
#endif

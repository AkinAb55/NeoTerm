/* uKernel — runtime kontroll API.
 *
 * Ezt a uServer hívja a kernel_shim.so-ban. A driver soha nem látja: ez a
 * shim "kívülről vezérlő" felülete (modul-életciklus, USB enumeráció). */
#ifndef UKERNEL_RUNTIME_H
#define UKERNEL_RUNTIME_H

#include <stddef.h>
#include "ukernel/hcd.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Egy regisztrált kernelmodul (module_init/module_exit). */
struct ukernel_module {
	const char *name;
	int  (*init)(void);
	void (*exit)(void);
};

/* --- Modul-életciklus --- */
size_t ukernel_module_count(void);
const struct ukernel_module *ukernel_module_get(size_t i);
/* Lefuttatja az összes regisztrált module_init-et regisztrációs sorrendben.
 * Az első nemnulla visszatérést hibaként adja vissza. */
int ukernel_run_module_inits(void);
void ukernel_run_module_exits(void);

/* --- HCD kötés --- */
/* A shim usb_core ezután ezen a backenden keresztül beszél az eszközzel. */
void ukernel_set_hcd(const struct ukernel_hcd_ops *ops);

/* --- USB enumeráció + probe --- */
/* Megnyitja az eszközt a HCD-n, felépíti a usb_device-t a descriptorokból,
 * végigmegy a regisztrált usb_driver-eken, illeszti az id_table-t, és a
 * találatra meghívja a probe()-ot. Visszaad: hány eszközt sikerült bind-olni. */
int ukernel_usb_enumerate_and_probe(uint16_t vid, uint16_t pid);

/* Tiszta leállás: a pump-szál leállítása + a HCD lezárása (libusb release+close+exit). A bridge a
 * destruktorából hívja, hogy ne crasheljen a process-teardownkor és az eszközt tisztán hagyja. */
void ukernel_usb_shutdown(void);

/* Az utolsó enumerált eszköz USB-osztálya (a hotplug-daemon class-routingjához): interfész- ill.
 * eszköz-osztály. 0x08 = mass-storage, 0xe0 = wireless, 0x02/0x0a = CDC(serial/net), 0xff = vendor. */
int ukernel_usb_iface_class(void);
int ukernel_usb_dev_class(void);

/* A HCD-eszköz elengedése (release+close): a hotplug-daemon nem-soros eszközre hívja, hogy a
 * megfelelő osztály-shim (FS-bridge / net-bridge) közvetlenül megnyithassa. */
void ukernel_usb_close(void);

/* --- Proxy file-API (char-device) ---
 * A uServer ezekkel hívja a betöltött driver file_operations-ét a kliens
 * nevében. Driver-független: bármely register_chrdev/ukernel_register_cdev-vel
 * regisztrált eszköz elérhető. */
size_t      ukernel_cdev_count(void);
const char *ukernel_cdev_name(size_t idx);
void       *ukernel_file_open(const char *name, unsigned int flags);
long        ukernel_file_read(void *fh, void *buf, size_t n);
long        ukernel_file_write(void *fh, const void *buf, size_t n);
long        ukernel_file_ioctl(void *fh, unsigned int cmd, void *arg);
unsigned int ukernel_file_poll(void *fh);
void        ukernel_file_close(void *fh);

/* --- Logolás szint --- */
void ukernel_set_loglevel(int level); /* 0=hiba .. 7=debug */

#ifdef __cplusplus
}
#endif
#endif /* UKERNEL_RUNTIME_H */

/* NeoTerm USB (libusb) enablement — injected into proot's syscall/enter.c after
 * the block/fs/cam redirects. Gated by UK_USB.
 *
 * Goal: make UNMODIFIED distro libusb work under proot (no in-distro patched
 * libusb). On an Android device, stock libusb's init dies before it ever
 * enumerates:
 *
 *   [op_init] sysfs is available
 *   [linux_udev_start_event_monitor] could not initialize udev monitor
 *   [op_init] error starting hotplug event monitor
 *   unable to initialize libusb: -99
 *
 * libusb treats the hotplug (udev/netlink) monitor as fatal. The monitor is a
 * NETLINK_KOBJECT_UEVENT socket bound to a multicast group, which Android's
 * SELinux blocks for app-uid processes -> bind() fails -> libusb_init() == -99.
 *
 * Fix without touching libusb: fake success for exactly that bind() — an
 * AF_NETLINK socket binding to a non-zero multicast group. The monitor then
 * "starts" (events simply never arrive — fine, no kernel uevents under proot
 * anyway) and libusb_init() succeeds, after which enumeration proceeds via the
 * sysfs path that libusb already reports as available.
 *
 * Scope: only AF_NETLINK binds with nl_groups != 0 (i.e. monitors) are faked;
 * ordinary netlink request sockets (groups == 0) and all non-netlink binds pass
 * through untouched. This does mean a guest netlink *monitor* (e.g. `ip monitor`)
 * would also see a successful-but-silent bind; acceptable under the UK_USB gate.
 *
 * Not compiled standalone: uses enter.c's Tracee, peek_reg/poke_reg,
 * read_data, set_sysnum(PR_void), PR_bind, SYSARG_*, CURRENT.
 */
static int g_uk_usb = -1;
static int uk_usb_on(void)
{
	if (g_uk_usb < 0) { const char *e = getenv("UK_USB"); g_uk_usb = (e && *e && *e != '0') ? 1 : 0; }
	return g_uk_usb;
}

static bool uknl_usb_dispatch(Tracee *tracee, word_t nr)
{
	if (!uk_usb_on()) return false;
	if (nr != PR_bind) return false;
	word_t addr = peek_reg(tracee, CURRENT, SYSARG_2);
	word_t len  = peek_reg(tracee, CURRENT, SYSARG_3);
	if (!addr || len < 12) return false;            /* sockaddr_nl is 12 bytes */
	unsigned char sa[12];
	if (read_data(tracee, sa, addr, sizeof sa) < 0) return false;
	unsigned short fam = (unsigned short)(sa[0] | (sa[1] << 8));
	unsigned int groups = (unsigned)(sa[8] | (sa[9] << 8) | (sa[10] << 16) | (sa[11] << 24));
	if (fam != 16 /* AF_NETLINK */ || groups == 0) return false;
	/* Fake bind() success so libusb's udev hotplug monitor initialises. */
	poke_reg(tracee, SYSARG_RESULT, 0);
	set_sysnum(tracee, PR_void);
	return true;
}

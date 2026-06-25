/* uKernel hamis <scsi/scsi_device.h> — minimalis SCSI eszkoz/cel modell a
 * usb-storage + uas driverekhez. Csak a tenylegesen hivatkozott mezok/protok.
 * Referencia: /root/Projects/linux/include/scsi/scsi_device.h */
#ifndef _UK_SCSI_SCSI_DEVICE_H
#define _UK_SCSI_SCSI_DEVICE_H

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/list.h>

struct Scsi_Host;
struct request_queue;
struct scsi_target;

typedef __u64 blist_flags_t;

/* SCSI LUN — a UAS protokoll-IU-k es az int_to_scsilun() hasznalja. */
struct scsi_lun {
	__u8 scsi_lun[8];
};

/* eszkoztipusok (scsi_proto.h) */
#define TYPE_DISK	0x00
#define TYPE_TAPE	0x01
#define TYPE_PRINTER	0x02
#define TYPE_PROCESSOR	0x03
#define TYPE_WORM	0x04
#define TYPE_ROM	0x05
#define TYPE_SCANNER	0x06
#define TYPE_MOD	0x07
#define TYPE_MEDIUM_CHANGER 0x08
#define TYPE_RAID	0x0c
#define TYPE_ENCLOSURE	0x0d
#define TYPE_RBC	0x0e
#define TYPE_NO_LUN	0x7f

struct scsi_device {
	struct Scsi_Host	*host;
	struct request_queue	*request_queue;

	unsigned int		id, channel, lun;
	unsigned int		sector_size;

	void			*hostdata;	/* available to low-level driver */

	struct device		sdev_gendev;	/* a sysfs-attributumokhoz */

	char			type;
	char			scsi_level;
	unsigned char		inquiry_len;	/* valid bytes in INQUIRY */
	unsigned char		*inquiry;	/* INQUIRY response data */

	blist_flags_t		sdev_bflags;

	/* viselkedes-bitek, amelyeket a usb-storage allit */
	unsigned use_10_for_ms:1;
	unsigned use_192_bytes_for_3f:1;
	unsigned skip_ms_page_8:1;
	unsigned skip_ms_page_3f:1;
	unsigned skip_vpd_pages:1;
	unsigned try_vpd_pages:1;
	unsigned no_read_capacity_16:1;
	unsigned no_read_disc_info:1;
	unsigned no_report_opcodes:1;
	unsigned no_write_same:1;
	unsigned use_16_for_rw:1;
	unsigned fix_capacity:1;
	unsigned guess_capacity:1;
	unsigned retry_hwerror:1;
	unsigned last_sector_bug:1;
	unsigned wce_default_on:1;
	unsigned broken_fua:1;
	unsigned lockable:1;
	unsigned allow_restart:1;
	unsigned try_rc_10_first:1;
	unsigned read_before_ms:1;
};

struct scsi_target {
	struct device		dev;
	unsigned int		id, channel;
	unsigned int		pdt_1f_for_no_lun:1;
	unsigned int		no_report_luns:1;
};

/* BLIST_* eszkoz-info zaszlok (scsi_devinfo.h) — itt egyutt definialjuk. */
#define BLIST_FORCELUN		((__force blist_flags_t)(1ULL << 1))
#define BLIST_SKIP_IO_HINTS	((__force blist_flags_t)(1ULL << 47))

static inline int scsi_device_online(struct scsi_device *sdev) { (void)sdev; return 1; }

extern int scsi_change_queue_depth(struct scsi_device *sdev, int depth);

#define to_scsi_device(d)	container_of(d, struct scsi_device, sdev_gendev)

/* sdev_printk/scmd_printk — a userspace shimben egyszeru printk-ra kepezzuk. */
#define sdev_printk(level, sdev, fmt, ...)	\
	printk(level fmt, ##__VA_ARGS__)
#define scmd_printk(level, scmd, fmt, ...)	\
	printk(level fmt, ##__VA_ARGS__)
#define sdev_prefix_printk(level, sdev, name, fmt, ...)	\
	printk(level fmt, ##__VA_ARGS__)

#endif

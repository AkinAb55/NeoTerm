/* uKernel hamis <scsi/scsi_host.h> — struct Scsi_Host, scsi_host_template,
 * a host-eletciklus fuggvenyek (alloc/add/scan/remove/put) es DEF_SCSI_QCMD.
 * Referencia: /root/Projects/linux/include/scsi/scsi_host.h */
#ifndef _UK_SCSI_SCSI_HOST_H
#define _UK_SCSI_SCSI_HOST_H

#include <linux/types.h>
#include <linux/list.h>
#include <linux/device.h>
#include <linux/spinlock.h>
#include <scsi/scsi.h>
#include <scsi/scsi_cmnd.h>
#include <scsi/scsi_device.h>

struct module;
struct seq_file;
struct queue_limits;
struct scsi_target;
struct attribute_group;

struct scsi_host_template {
	const char		*name;
	const char		*proc_name;
	struct module		*module;

	const char		*(*info)(struct Scsi_Host *);
	int			(*show_info)(struct seq_file *, struct Scsi_Host *);
	int			(*write_info)(struct Scsi_Host *, char *, int);

	enum scsi_qc_status	(*queuecommand)(struct Scsi_Host *, struct scsi_cmnd *);

	int			(*eh_abort_handler)(struct scsi_cmnd *);
	int			(*eh_device_reset_handler)(struct scsi_cmnd *);
	int			(*eh_bus_reset_handler)(struct scsi_cmnd *);
	int			(*eh_host_reset_handler)(struct scsi_cmnd *);

	int			(*sdev_init)(struct scsi_device *);
	int			(*sdev_configure)(struct scsi_device *, struct queue_limits *);
	int			(*target_alloc)(struct scsi_target *);

	const struct attribute_group **sdev_groups;

	int			can_queue;
	int			this_id;
	unsigned short		sg_tablesize;
	unsigned int		max_sectors;
	unsigned int		max_hw_sectors;
	unsigned long		dma_boundary;
	unsigned long		dma_alignment;
	int			cmd_size;

	unsigned		emulated:1;
	unsigned		skip_settle_delay:1;

	/* a uas a SHT-t egyben usb_driver-kent is hasznalja a .name-en kivul */
	void			*driver_info;
};

struct Scsi_Host {
	struct list_head	__devices;

	spinlock_t		default_lock;
	spinlock_t		*host_lock;

	const struct scsi_host_template *hostt;

	unsigned int		host_no;
	int			can_queue;
	short			cmd_per_lun;
	unsigned short		sg_tablesize;
	unsigned short		sg_prot_tablesize;
	unsigned int		max_sectors;
	unsigned int		max_id;
	unsigned int		max_lun;
	unsigned int		max_channel;
	unsigned int		this_id;
	unsigned short		max_cmd_len;

	unsigned		no_scsi2_lun_in_cdb:1;
	unsigned		host_self_blocked:1;

	unsigned int		unique_id;

	struct device		shost_gendev;

	/* hostdata[0] — a tenyleges privat adat (us_data / uas_dev_info) ide kerul. */
	unsigned long		hostdata[0] __attribute__((aligned(sizeof(unsigned long))));
};

extern struct Scsi_Host *scsi_host_alloc(const struct scsi_host_template *, int);
extern int scsi_add_host(struct Scsi_Host *, struct device *);
extern void scsi_scan_host(struct Scsi_Host *);
extern void scsi_remove_host(struct Scsi_Host *);
extern void scsi_host_put(struct Scsi_Host *);
extern struct Scsi_Host *scsi_host_get(struct Scsi_Host *);

static inline void scsi_assign_lock(struct Scsi_Host *shost, spinlock_t *lock)
{ shost->host_lock = lock; }

#define dev_to_shost(d)		container_of(d, struct Scsi_Host, shost_gendev)
#define class_to_shost(d)	dev_to_shost(d)

#define shost_printk(prefix, shost, fmt, ...)	\
	printk(prefix fmt, ##__VA_ARGS__)

extern void scsi_block_requests(struct Scsi_Host *);
extern void scsi_unblock_requests(struct Scsi_Host *);

/*
 * DEF_SCSI_QCMD — a tenyleges kernelben a host_lock-ot fogja queuecommand_lck
 * korul. A userspace shimben a spin_lock no-op, de a szerkezetet megtartjuk.
 */
#define DEF_SCSI_QCMD(func_name) \
	enum scsi_qc_status func_name(struct Scsi_Host *shost,		\
				      struct scsi_cmnd *cmd)		\
	{								\
		unsigned long irq_flags;				\
		enum scsi_qc_status rc;					\
		spin_lock_irqsave(shost->host_lock, irq_flags);		\
		rc = func_name##_lck(cmd);				\
		spin_unlock_irqrestore(shost->host_lock, irq_flags);	\
		return rc;						\
	}

#endif

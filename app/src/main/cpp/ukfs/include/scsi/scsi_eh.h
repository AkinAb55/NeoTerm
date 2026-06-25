/* uKernel hamis <scsi/scsi_eh.h> — EH segedek a usb-storage transport.c-hez.
 * Referencia: /root/Projects/linux/include/scsi/scsi_eh.h */
#ifndef _UK_SCSI_SCSI_EH_H
#define _UK_SCSI_SCSI_EH_H

#include <linux/types.h>
#include <linux/scatterlist.h>
#include <scsi/scsi_cmnd.h>
#include <scsi/scsi_common.h>

struct scsi_device;
struct Scsi_Host;

extern void scsi_report_bus_reset(struct Scsi_Host *, int);
extern void scsi_report_device_reset(struct Scsi_Host *, int, int);

struct scsi_eh_save {
	int			result;
	unsigned int		resid_len;
	int			eh_eflags;
	enum dma_data_direction	data_direction;
	unsigned		underflow;
	unsigned char		cmd_len;
	unsigned char		prot_op;
	unsigned char		cmnd[32];
	struct scsi_data_buffer	sdb;
	struct scatterlist	sense_sgl;
};

extern void scsi_eh_prep_cmnd(struct scsi_cmnd *scmd, struct scsi_eh_save *ses,
			      unsigned char *cmnd, int cmnd_size,
			      unsigned sense_bytes);
extern void scsi_eh_restore_cmnd(struct scsi_cmnd *scmd, struct scsi_eh_save *ses);

#endif

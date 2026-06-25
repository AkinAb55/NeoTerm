/* uKernel hamis <scsi/scsi_cmnd.h> — a usb-storage/uas altal hasznalt
 * struct scsi_cmnd + scsi_data_buffer + sg/resid segedek.
 * Referencia: /root/Projects/linux/include/scsi/scsi_cmnd.h */
#ifndef _UK_SCSI_SCSI_CMND_H
#define _UK_SCSI_SCSI_CMND_H

#include <linux/types.h>
#include <linux/list.h>
#include <linux/scatterlist.h>
#include <linux/dma-mapping.h>
#include <scsi/scsi_device.h>

struct Scsi_Host;
struct request;

#define MAX_COMMAND_SIZE	16
#define SCSI_SENSE_BUFFERSIZE	96

struct scsi_data_buffer {
	struct sg_table		table;
	unsigned int		length;
};

/* scmd->flags */
#define SCMD_TAGGED		(1 << 0)
#define SCMD_INITIALIZED	(1 << 1)
#define SCMD_LAST		(1 << 2)

struct scsi_cmnd {
	struct scsi_device	*device;
	struct list_head	eh_entry;

	unsigned long		jiffies_at_alloc;

	int			retries;
	int			allowed;

	unsigned char		prot_op;
	unsigned char		prot_type;
	unsigned char		prot_flags;

	unsigned short		cmd_len;
	enum dma_data_direction	sc_data_direction;

	unsigned char		cmnd[32];	/* SCSI CDB */

	struct scsi_data_buffer	sdb;

	unsigned		underflow;
	unsigned		transfersize;
	unsigned		resid_len;
	unsigned		sense_len;
	unsigned char		*sense_buffer;

	int			flags;
	unsigned long		state;

	struct request		*request;

	unsigned char		*host_scribble;

	int			result;
};

static inline void *scsi_cmd_priv(struct scsi_cmnd *cmd) { return cmd + 1; }

void scsi_done(struct scsi_cmnd *cmd);
void scsi_done_direct(struct scsi_cmnd *cmd);

static inline unsigned scsi_sg_count(struct scsi_cmnd *cmd)
{ return cmd->sdb.table.nents; }
static inline struct scatterlist *scsi_sglist(struct scsi_cmnd *cmd)
{ return cmd->sdb.table.sgl; }
static inline unsigned scsi_bufflen(struct scsi_cmnd *cmd)
{ return cmd->sdb.length; }
static inline void scsi_set_resid(struct scsi_cmnd *cmd, unsigned int resid)
{ cmd->resid_len = resid; }
static inline unsigned int scsi_get_resid(struct scsi_cmnd *cmd)
{ return cmd->resid_len; }

#define scsi_for_each_sg(cmd, sg, nseg, __i) \
	for_each_sg(scsi_sglist(cmd), sg, nseg, __i)

static inline struct request *scsi_cmd_to_rq(struct scsi_cmnd *scmd)
{ return scmd->request; }

static inline void set_host_byte(struct scsi_cmnd *cmd, char status)
{ cmd->result = (cmd->result & 0xff00ffff) | (status << 16); }
static inline u8 get_host_byte(struct scsi_cmnd *cmd)
{ return (cmd->result >> 16) & 0xff; }
static inline void set_status_byte(struct scsi_cmnd *cmd, char status)
{ cmd->result = (cmd->result & 0xffffff00) | status; }
static inline u8 get_status_byte(struct scsi_cmnd *cmd)
{ return cmd->result & 0xff; }

extern void scsi_build_sense(struct scsi_cmnd *scmd, int desc,
			     u8 key, u8 asc, u8 ascq);

#endif

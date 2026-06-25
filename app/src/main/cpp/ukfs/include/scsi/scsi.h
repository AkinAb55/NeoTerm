/* uKernel hamis <scsi/scsi.h> — a usb-storage altal hivatkozott SCSI
 * parancs-opcode-ok, status/sense konstansok, disposition/qc-status enumok es
 * a scsi_command_size/scsi_status_is_good segedek.
 * Referencia: /root/Projects/linux/include/scsi/{scsi.h,scsi_proto.h,scsi_status.h,scsi_common.h} */
#ifndef _UK_SCSI_SCSI_H
#define _UK_SCSI_SCSI_H

#include <linux/types.h>

struct scsi_cmnd;
struct scsi_device;
struct scsi_lun;

/* ---- SCSI parancs-opcode-ok (scsi_proto.h) ---- */
#define TEST_UNIT_READY		0x00
#define REZERO_UNIT		0x01
#define REQUEST_SENSE		0x03
#define READ_6			0x08
#define WRITE_6			0x0a
#define INQUIRY			0x12
#define MODE_SELECT		0x15
#define RESERVE			0x16
#define RELEASE			0x17
#define MODE_SENSE		0x1a
#define START_STOP		0x1b
#define SEND_DIAGNOSTIC		0x1d
#define ALLOW_MEDIUM_REMOVAL	0x1e
#define SET_WINDOW		0x24
#define READ_CAPACITY		0x25
#define READ_10			0x28
#define WRITE_10		0x2a
#define WRITE_VERIFY		0x2e
#define VERIFY			0x2f
#define SYNCHRONIZE_CACHE	0x35
#define READ_TOC		0x43
#define MODE_SELECT_10		0x55
#define MODE_SENSE_10		0x5a
#define PERSISTENT_RESERVE_IN	0x5e
#define PERSISTENT_RESERVE_OUT	0x5f
#define VARIABLE_LENGTH_CMD	0x7f
#define EXTENDED_COPY		0x83
#define RECEIVE_COPY_RESULTS	0x84
#define REPORT_LUNS		0xa0
#define MAINTENANCE_IN		0xa3
#define MAINTENANCE_OUT		0xa4
#define READ_12			0xa8
#define SERVICE_ACTION_OUT_12	0xa9
#define WRITE_12		0xaa
#define SERVICE_ACTION_IN_12	0xab
#define SERVICE_ACTION_BIDIRECTIONAL 0x9d
#define SERVICE_ACTION_IN_16	0x9e
#define SERVICE_ACTION_OUT_16	0x9f
#define ATA_16			0x85	/* 16-byte ATA pass-thru */
#define ATA_12			0xa1	/* 12-byte ATA pass-thru */
#define LOG_SENSE		0x4d
/* a kernel sd.h scsi_medium_access_command()-ja altal hivatkozott tovabbi opcode-ok */
#define READ_16			0x88
#define WRITE_16		0x8a
#define VERIFY_16		0x8f
#define WRITE_SAME_16		0x93
#define READ_32			0x09
#define VERIFY_32		0x0a
#define WRITE_32		0x0b
#define WRITE_SAME_32		0x0d
#define WRITE_SAME		0x41
#define UNMAP			0x42
#define VERIFY_12		0xaf

/* ---- SCSI uzenet-bajtok ---- */
#define COMMAND_COMPLETE	0x00
#define ABORT_TASK_SET		0x06
#define TARGET_RESET		0x0c

/* ---- sense kulcsok ---- */
#define NO_SENSE		0x00
#define RECOVERED_ERROR		0x01
#define NOT_READY		0x02
#define MEDIUM_ERROR		0x03
#define HARDWARE_ERROR		0x04
#define ILLEGAL_REQUEST		0x05
#define UNIT_ATTENTION		0x06
#define DATA_PROTECT		0x07
#define BLANK_CHECK		0x08
#define VENDOR_SPECIFIC		0x09
#define COPY_ABORTED		0x0a
#define ABORTED_COMMAND		0x0b
#define VOLUME_OVERFLOW		0x0d
#define MISCOMPARE		0x0e

/* ---- SAM status (scsi_status.h) ---- */
#define SAM_STAT_GOOD			0x00
#define SAM_STAT_CHECK_CONDITION	0x02
#define SAM_STAT_CONDITION_MET		0x04
#define SAM_STAT_BUSY			0x08
#define SAM_STAT_RESERVATION_CONFLICT	0x18
#define SAM_STAT_COMMAND_TERMINATED	0x22
#define SAM_STAT_TASK_SET_FULL		0x28
#define SAM_STAT_TASK_ABORTED		0x40

/* ---- host (DID_*) status (scsi_status.h) ---- */
enum scsi_host_status {
	DID_OK		= 0x00,
	DID_NO_CONNECT	= 0x01,
	DID_BUS_BUSY	= 0x02,
	DID_TIME_OUT	= 0x03,
	DID_BAD_TARGET	= 0x04,
	DID_ABORT	= 0x05,
	DID_PARITY	= 0x06,
	DID_ERROR	= 0x07,
	DID_RESET	= 0x08,
	DID_BAD_INTR	= 0x09,
	DID_PASSTHROUGH	= 0x0a,
	DID_SOFT_ERROR	= 0x0b,
	DID_IMM_RETRY	= 0x0c,
	DID_REQUEUE	= 0x0d,
	DID_TRANSPORT_DISRUPTED = 0x0e,
	DID_TRANSPORT_FAILFAST = 0x0f,
};

/* ---- EH disposition / qc-status ---- */
enum scsi_disposition {
	NEEDS_RETRY	= 0x2001,
	SUCCESS		= 0x2002,
	FAILED		= 0x2003,
	QUEUED		= 0x2004,
	SOFT_ERROR	= 0x2005,
	ADD_TO_MLQUEUE	= 0x2006,
	TIMEOUT_ERROR	= 0x2007,
	SCSI_RETURN_NOT_HANDLED	= 0x2008,
	FAST_IO_FAIL	= 0x2009,
};

enum scsi_qc_status {
	SCSI_MLQUEUE_HOST_BUSY   = 0x1055,
	SCSI_MLQUEUE_DEVICE_BUSY = 0x1056,
	SCSI_MLQUEUE_EH_RETRY    = 0x1057,
	SCSI_MLQUEUE_TARGET_BUSY = 0x1058,
};

/* ---- scsi_level (struct scsi_device::scsi_level) ---- */
#define SCSI_UNKNOWN	0
#define SCSI_1		1
#define SCSI_1_CCS	2
#define SCSI_2		3
#define SCSI_3		4
#define SCSI_SPC_2	5
#define SCSI_SPC_3	6

#define SCSI_MAX_SG_SEGMENTS	128

#define SCAN_WILD_CARD	~0

#define MAX_COMMAND_SIZE	16
#define SCSI_SENSE_BUFFERSIZE	96

/* ---- result-bajt segedek ---- */
#define status_byte(result)	((result) & 0xff)
#define host_byte(result)	(((result) >> 16) & 0xff)
#define sense_class(sense)	(((sense) >> 4) & 0x7)
#define sense_error(sense)	((sense) & 0xf)
#define sense_valid(sense)	((sense) & 0x80)

/* SCSI_LUN — a usb-storage a host_byte/result epitesehez hasznalja (lasd usb.c) */
#ifndef SCSI_LUN
#define SCSI_LUN(cmnd)	((cmnd)->device->lun)
#endif

static inline int scsi_status_is_check_condition(int status)
{
	if (status < 0)
		return false;
	status &= 0xfe;
	return status == SAM_STAT_CHECK_CONDITION;
}

static inline bool scsi_status_is_good(int status)
{
	if (status < 0)
		return false;
	status &= 0xfe;
	return status == SAM_STAT_GOOD ||
	       status == SAM_STAT_CONDITION_MET;
}

/* ---- scsi_command_size (scsi_common.h) ---- */
extern const unsigned char scsi_command_size_tbl[8];
#define COMMAND_SIZE(opcode) scsi_command_size_tbl[((opcode) >> 5) & 7]

static inline unsigned scsi_command_size(const unsigned char *cmnd)
{
	return (cmnd[0] == VARIABLE_LENGTH_CMD) ?
		(((const u8 *)cmnd)[7] + 8) : COMMAND_SIZE(cmnd[0]);
}

extern void int_to_scsilun(u64 lun, struct scsi_lun *scsilun);
extern u64 scsilun_to_int(struct scsi_lun *scsilun);

#endif

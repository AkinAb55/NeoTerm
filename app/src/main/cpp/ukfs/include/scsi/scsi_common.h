/* uKernel hamis <scsi/scsi_common.h> — scsi_sense_hdr + sense segedek.
 * Referencia: /root/Projects/linux/include/scsi/scsi_common.h */
#ifndef _UK_SCSI_COMMON_H
#define _UK_SCSI_COMMON_H

#include <linux/types.h>
#include <scsi/scsi.h>

struct scsi_sense_hdr {
	u8 response_code;
	u8 sense_key;
	u8 asc;
	u8 ascq;
	u8 byte4;
	u8 byte5;
	u8 byte6;
	u8 additional_length;
};

static inline bool scsi_sense_valid(const struct scsi_sense_hdr *sshdr)
{
	if (!sshdr)
		return false;
	return (sshdr->response_code & 0x70) == 0x70;
}

extern bool scsi_normalize_sense(const u8 *sense_buffer, int sb_len,
				 struct scsi_sense_hdr *sshdr);
extern const u8 *scsi_sense_desc_find(const u8 *sense_buffer, int sb_len,
				      int desc_type);
extern void scsi_build_sense_buffer(int desc, u8 *buf, u8 key, u8 asc, u8 ascq);
extern const char *scsi_device_type(unsigned type);

#endif

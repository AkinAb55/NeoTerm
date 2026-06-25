/* uKernel hamis <scsi/scsi_dbg.h> — SCSI debug-print protok.
 * Referencia: /root/Projects/linux/include/scsi/scsi_dbg.h */
#ifndef _UK_SCSI_SCSI_DBG_H
#define _UK_SCSI_SCSI_DBG_H

struct scsi_cmnd;
struct scsi_device;
struct scsi_sense_hdr;

extern void scsi_print_command(struct scsi_cmnd *);
extern void scsi_print_sense_hdr(const struct scsi_device *, const char *,
				 const struct scsi_sense_hdr *);
extern void scsi_print_sense(struct scsi_cmnd *);
extern void scsi_print_result(struct scsi_cmnd *, const char *, int);

extern const char *scsi_sense_key_string(unsigned char);
extern const char *scsi_extd_sense_format(unsigned char, unsigned char,
					  const char **);
extern const char *scsi_mlreturn_string(int);
extern const char *scsi_hostbyte_string(int);

#endif

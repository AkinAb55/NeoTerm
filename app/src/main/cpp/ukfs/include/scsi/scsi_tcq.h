/* uKernel hamis <scsi/scsi_tcq.h> — a uas csak a scsi_change_queue_depth-et
 * hivja, ami a <scsi/scsi_device.h>-ban van deklaralva. */
#ifndef _UK_SCSI_SCSI_TCQ_H
#define _UK_SCSI_SCSI_TCQ_H
#include <scsi/scsi_device.h>
#include <scsi/scsi_cmnd.h>
#endif

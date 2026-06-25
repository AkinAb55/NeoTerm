/* uKernel hamis <linux/cdrom.h> — csak a usb-storage altal hivatkozott
 * GPCMD_* MMC parancs-opcode-ok. */
#ifndef _UK_LINUX_CDROM_H
#define _UK_LINUX_CDROM_H

#define GPCMD_BLANK				0xa1
#define GPCMD_CLOSE_TRACK			0x5b
#define GPCMD_GET_EVENT_STATUS_NOTIFICATION	0x4a
#define GPCMD_MECHANISM_STATUS			0xbd
#define GPCMD_PAUSE_RESUME			0x4b
#define GPCMD_PLAY_AUDIO_MSF			0x47
#define GPCMD_READ_CD				0xbe
#define GPCMD_READ_CD_MSF			0xb9
#define GPCMD_READ_DISC_INFO			0x51
#define GPCMD_READ_HEADER			0x44
#define GPCMD_READ_SUBCHANNEL			0x42
#define GPCMD_READ_TRACK_RZONE_INFO		0x52
#define GPCMD_REPAIR_RZONE_TRACK		0x58
#define GPCMD_RESERVE_RZONE_TRACK		0x53
#define GPCMD_SCAN				0xba
#define GPCMD_SEND_OPC				0x54
#define GPCMD_SET_SPEED				0xbb
#define GPCMD_STOP_PLAY_SCAN			0x4e

#endif

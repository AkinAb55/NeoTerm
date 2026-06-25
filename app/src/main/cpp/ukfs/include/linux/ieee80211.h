/* uKernel hamis <linux/ieee80211.h> — 802.11 keret-konstansok és segédek. */
#ifndef _UK_LINUX_IEEE80211_H
#define _UK_LINUX_IEEE80211_H

#include <linux/types.h>

#define IEEE80211_FCTL_FTYPE      0x000c
#define IEEE80211_FCTL_STYPE      0x00f0
#define IEEE80211_FTYPE_MGMT      0x0000
#define IEEE80211_FTYPE_CTL       0x0004
#define IEEE80211_FTYPE_DATA      0x0008
#define IEEE80211_STYPE_BEACON    0x0080
#define IEEE80211_STYPE_ASSOC_REQ   0x0000
#define IEEE80211_STYPE_ASSOC_RESP  0x0010
#define IEEE80211_STYPE_REASSOC_REQ 0x0020
#define IEEE80211_STYPE_REASSOC_RESP 0x0030
#define IEEE80211_STYPE_PROBE_REQ   0x0040
#define IEEE80211_STYPE_PROBE_RESP  0x0050
#define IEEE80211_STYPE_AUTH        0x00B0
#define IEEE80211_STYPE_DEAUTH      0x00C0
#define IEEE80211_STYPE_DISASSOC    0x00A0
#define IEEE80211_STYPE_ACTION      0x00D0
#define IEEE80211_HT_CAP_SUP_WIDTH_20_40 0x0002
#define IEEE80211_HT_MAX_AMPDU_64K   3
#define IEEE80211_HT_MPDU_DENSITY_16 7
#define IEEE80211_HT_MCS_TX_DEFINED  0x01

#define ETH_ALEN  6
#define IEEE80211_MAX_SSID_LEN 32

struct ieee80211_hdr {
	__le16 frame_control;
	__le16 duration_id;
	u8     addr1[ETH_ALEN];
	u8     addr2[ETH_ALEN];
	u8     addr3[ETH_ALEN];
	__le16 seq_ctrl;
} __attribute__((packed));

int ieee80211_get_hdrlen(u16 fc);
int ieee80211_is_empty_essid(const char *essid, int len);

/* csatorna <-> frekvencia (2.4 és 5 GHz) */
int ieee80211_channel_to_frequency(int chan, int band);
int ieee80211_frequency_to_channel(int freq);

#endif

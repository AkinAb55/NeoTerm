/* uKernel hamis <net/cfg80211.h> — a vezeték nélküli stack driver-felülete.
 * A from-scratch shim filozófiával, a valós rtl8812au cfg80211-glue (ioctl_cfg80211.c)
 * által ténylegesen használt felületre szabva. A cfg80211_ops a valódi kernel
 * szignatúráival (a hivatkozott típusok opak forward-deklarálva). */
#ifndef _UK_NET_CFG80211_H
#define _UK_NET_CFG80211_H

#include <linux/types.h>
#include <linux/netdevice.h>
#include <linux/nl80211.h>
#include <linux/ieee80211.h>
#include <net/regulatory.h>

struct wiphy;
struct cfg80211_ops;
struct ieee80211_mgmt;

/* ===== konstansok ===== */
#define WLAN_PMKID_LEN 16
#define WLAN_MAX_KEY_LEN 32
#define WLAN_CIPHER_SUITE_USE_GROUP  0x000FAC00
#define WLAN_CIPHER_SUITE_WEP40      0x000FAC01
#define WLAN_CIPHER_SUITE_TKIP       0x000FAC02
#define WLAN_CIPHER_SUITE_CCMP       0x000FAC04
#define WLAN_CIPHER_SUITE_WEP104     0x000FAC05
#define WLAN_CIPHER_SUITE_AES_CMAC   0x000FAC06
#define WLAN_CIPHER_SUITE_GCMP       0x000FAC08
#define WLAN_CIPHER_SUITE_CCMP_256   0x000FAC0A
#define WLAN_CIPHER_SUITE_SMS4       0x00147201
#define IW_AUTH_CIPHER_NONE   0x01
#define IW_AUTH_CIPHER_WEP40  0x02
#define IW_AUTH_CIPHER_TKIP   0x04
#define IW_AUTH_CIPHER_CCMP   0x08
#define IW_AUTH_CIPHER_WEP104 0x10
#define WLAN_AKM_SUITE_8021X   0x000FAC01
#define WLAN_AKM_SUITE_PSK     0x000FAC02
#define WLAN_AKM_SUITE_SAE     0x000FAC08
enum ieee80211_bss_type { IEEE80211_BSS_TYPE_ESS, IEEE80211_BSS_TYPE_IBSS, IEEE80211_BSS_TYPE_ANY };
enum ieee80211_privacy { IEEE80211_PRIVACY_ON, IEEE80211_PRIVACY_OFF, IEEE80211_PRIVACY_ANY };

/* ===== csatorna / sáv ===== */
enum ieee80211_channel_flags {
	IEEE80211_CHAN_DISABLED      = 1 << 0,
	IEEE80211_CHAN_NO_IR         = 1 << 1,
	IEEE80211_CHAN_RADAR         = 1 << 3,
	IEEE80211_CHAN_NO_HT40PLUS   = 1 << 4,
	IEEE80211_CHAN_NO_HT40MINUS  = 1 << 5,
	IEEE80211_CHAN_NO_80MHZ      = 1 << 7,
	IEEE80211_CHAN_NO_160MHZ     = 1 << 8,
};
#define IEEE80211_CHAN_NO_HT40 (IEEE80211_CHAN_NO_HT40PLUS | IEEE80211_CHAN_NO_HT40MINUS)

struct ieee80211_channel {
	enum nl80211_band band;
	u32  center_freq;
	u16  freq_offset;
	u16  hw_value;
	u32  flags;
	int  max_antenna_gain;
	int  max_power;
	int  max_reg_power;
	bool beacon_found;
	u32  orig_flags;
	int  orig_mag, orig_mpwr;
	int  dfs_state;
	unsigned int dfs_cac_ms;
};

struct ieee80211_rate { u32 flags; u16 bitrate; u16 hw_value, hw_value_short; };

struct ieee80211_mcs_info { u8 rx_mask[10]; u16 rx_highest; u8 tx_params; u8 reserved[3]; };
struct ieee80211_vht_mcs_info { u16 rx_mcs_map; u16 rx_highest; u16 tx_mcs_map; u16 tx_highest; };

struct ieee80211_sta_ht_cap {
	u16 cap;
	bool ht_supported;
	u8 ampdu_factor, ampdu_density;
	struct ieee80211_mcs_info mcs;
};
struct ieee80211_sta_vht_cap {
	bool vht_supported;
	u32 cap;
	struct ieee80211_vht_mcs_info vht_mcs;
};

struct ieee80211_supported_band {
	struct ieee80211_channel *channels;
	struct ieee80211_rate    *bitrates;
	enum nl80211_band band;
	int n_channels;
	int n_bitrates;
	struct ieee80211_sta_ht_cap ht_cap;
	struct ieee80211_sta_vht_cap vht_cap;
};

/* ===== wiphy / wireless_dev ===== */
enum cfg80211_signal_type { CFG80211_SIGNAL_TYPE_NONE, CFG80211_SIGNAL_TYPE_MBM, CFG80211_SIGNAL_TYPE_UNSPEC };

struct wiphy {
	const struct cfg80211_ops *ops;
	struct ieee80211_supported_band *bands[NUM_NL80211_BANDS];
	u8    perm_addr[6];
	const void *mgmt_stypes;
	const void *iface_combinations;
	int   n_iface_combinations;
	u16   software_iftypes;
	u16   interface_modes;
	u16   max_acl_mac_addrs;
	u32   flags, regulatory_flags, features;
	enum cfg80211_signal_type signal_type;
	u8    max_scan_ssids;
	u8    max_sched_scan_reqs;
	u8    max_sched_scan_ssids;
	u8    max_match_sets;
	u16   max_scan_ie_len;
	u16   max_sched_scan_ie_len;
	int   n_cipher_suites;
	const u32 *cipher_suites;
	u8    retry_short, retry_long;
	u32   frag_threshold, rts_threshold;
	const void *wowlan;
	u16   max_remain_on_channel_duration;
	u8    max_num_pmkids;
	u32   available_antennas_tx, available_antennas_rx;
	void (*reg_notifier)(struct wiphy *wiphy, struct regulatory_request *request);
	struct device dev;
	char  fw_version[32];
	void *priv_area;
	char  priv[];
};
struct regulatory_request { char alpha2[2]; int initiator; };

#define WIPHY_FLAG_NETNS_OK              (1<<0)
#define WIPHY_FLAG_PS_ON_BY_DEFAULT      (1<<1)
#define WIPHY_FLAG_4ADDR_AP              (1<<2)
#define WIPHY_FLAG_4ADDR_STATION         (1<<3)
#define WIPHY_FLAG_CONTROL_PORT_PROTOCOL (1<<4)
#define WIPHY_FLAG_HAVE_AP_SME           (1<<5)
#define WIPHY_FLAG_REPORTS_OBSS          (1<<6)
#define WIPHY_FLAG_AP_PROBE_RESP_OFFLOAD (1<<7)
#define WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL (1<<8)
#define WIPHY_FLAG_SUPPORTS_FW_ROAM      (1<<9)
#define WIPHY_FLAG_HAS_CHANNEL_SWITCH    (1<<10)
#define WIPHY_WOWLAN_ANY                 (1<<1)
struct cfg80211_wowlan { bool any, disconnect, magic_pkt; int n_patterns; };

static inline void *wiphy_priv(struct wiphy *wiphy) { return (void *)wiphy->priv; }
static inline struct device *wiphy_dev(struct wiphy *wiphy) { return &wiphy->dev; }
#define set_wiphy_dev(wiphy, d) do { (wiphy)->dev.parent = (d); } while (0)
const char *wiphy_name(struct wiphy *wiphy);

struct wireless_dev {
	struct wiphy *wiphy;
	struct net_device *netdev;
	enum nl80211_iftype iftype;
	u8    address[6];
	struct cfg80211_chan_def *preset_chandef;
	u8    ssid[IEEE80211_MAX_SSID_LEN];
	u8    ssid_len;
	void *current_bss;
	bool connected;
	struct { struct { void *current_bss; } client; } links[1];
};

/* ===== scan / bss ===== */
struct cfg80211_ssid { u8 ssid[IEEE80211_MAX_SSID_LEN]; u8 ssid_len; };

struct cfg80211_scan_request {
	struct cfg80211_ssid *ssids;
	int n_ssids;
	u32 n_channels;
	const u8 *ie;
	size_t ie_len;
	struct wiphy *wiphy;
	struct wireless_dev *wdev;
	struct ieee80211_channel *channels[];
};
struct cfg80211_scan_info { u64 scan_start_tsf; bool aborted; };

struct cfg80211_bss {
	struct ieee80211_channel *channel;
	u8   bssid[6];
	s32  signal;
	u16  beacon_interval;
	u16  capability;
	const u8 *ies;
	size_t ies_len;
	u8   ssid[IEEE80211_MAX_SSID_LEN];
	u8   ssid_len;
};
struct cfg80211_inform_bss { struct ieee80211_channel *chan; s32 signal; };

/* ===== param-structok (a driver dereferálja ezeket) ===== */
struct cfg80211_chan_def {
	struct ieee80211_channel *chan;
	enum nl80211_chan_width width;
	u32 center_freq1, center_freq2;
};
struct key_params {
	const u8 *key;
	const u8 *seq;
	int key_len, seq_len;
	u32 cipher;
};
struct cfg80211_crypto_settings {
	u32 wpa_versions;
	u32 cipher_group;
	int n_ciphers_pairwise;
	u32 ciphers_pairwise[5];
	int n_akm_suites;
	u32 akm_suites[2];
	bool control_port;
	__be16 control_port_ethertype;
	bool control_port_no_encrypt;
};
struct rate_info { u8 flags; u8 mcs; u16 legacy; u8 nss; u8 bw; };
struct sta_bss_parameters { u8 flags; u8 dtim_period; u16 beacon_interval; };
struct station_info {
	u64 filled;
	u32 connected_time;
	u32 inactive_time;
	u64 rx_bytes, tx_bytes;
	s8  signal, signal_avg;
	struct rate_info txrate, rxrate;
	u32 rx_packets, tx_packets, tx_retries, tx_failed, rx_dropped_misc;
	struct sta_bss_parameters bss_param;
	struct nl80211_sta_flag_update sta_flags;
	int generation;
	const u8 *assoc_req_ies;
	size_t assoc_req_ies_len;
	u16 llid, plid;
	u8  plink_state;
	u32 local_pm, peer_pm, nonpeer_pm;
};
struct station_parameters {
	const u8 *supported_rates;
	int  supported_rates_len;
	u16  aid;
	u16  listen_interval;
	u16  capability;
	const u8 *ext_capab;
	u8   ext_capab_len;
	struct nl80211_sta_flag_update sta_flags_set, sta_flags_mask;
	const struct ieee80211_ht_cap *ht_capa;
	const struct ieee80211_vht_cap *vht_capa;
	u8   plink_action, plink_state;
	u8   uapsd_queues;
	u8   max_sp;
};
struct cfg80211_connect_params {
	struct ieee80211_channel *channel;
	const u8 *bssid;
	const u8 *ssid;
	size_t ssid_len;
	enum nl80211_auth_type auth_type;
	const u8 *ie;
	size_t ie_len;
	bool privacy;
	enum nl80211_mfp mfp;
	struct cfg80211_crypto_settings crypto;
	const u8 *key;
	u8  key_len, key_idx;
	u32 flags;
};
struct cfg80211_beacon_data {
	const u8 *head, *tail;
	const u8 *beacon_ies, *proberesp_ies, *assocresp_ies, *probe_resp;
	size_t head_len, tail_len;
	size_t beacon_ies_len, proberesp_ies_len, assocresp_ies_len, probe_resp_len;
};
struct cfg80211_ap_settings {
	struct cfg80211_chan_def chandef;
	struct cfg80211_beacon_data beacon;
	int beacon_interval, dtim_period;
	const u8 *ssid;
	size_t ssid_len;
	enum nl80211_hidden_ssid hidden_ssid;
	struct cfg80211_crypto_settings crypto;
	bool privacy;
	enum nl80211_auth_type auth_type;
	int inactivity_timeout;
};
struct cfg80211_ibss_params {
	const u8 *ssid;
	const u8 *bssid;
	struct cfg80211_chan_def chandef;
	const u8 *ie;
	u8 ssid_len, ie_len;
	u16 beacon_interval;
	u32 basic_rates;
	bool channel_fixed, privacy;
};
struct ieee80211_txq_params {
	enum nl80211_ac ac;
	u16 txop, cwmin, cwmax;
	u8  aifs;
};
struct cfg80211_pmksa { const u8 *bssid; const u8 *pmkid; const u8 *ssid; const u8 *pmk; size_t pmk_len; };
struct cfg80211_mgmt_tx_params {
	struct ieee80211_channel *chan;
	bool offchan;
	unsigned int wait;
	const u8 *buf;
	size_t len;
	bool no_cck, dont_wait_for_ack;
};
struct cfg80211_acl_data {
	enum nl80211_acl_policy acl_policy;
	int n_acl_entries;
	struct { u8 addr[6]; } mac_addrs[];
};
struct survey_info {
	struct ieee80211_channel *channel;
	u64 filled;
	s8  noise;
	u64 time, time_busy, time_rx, time_tx;
};
struct vif_params { int use_4addr; const u8 *macaddr; };

struct wiphy_wowlan_support { u32 flags; int n_patterns; int pattern_max_len, pattern_min_len, max_pkt_offset; };
struct ieee80211_txrx_stypes { u16 tx, rx; };
struct ieee80211_iface_limit { u16 max; u16 types; };
struct ieee80211_iface_combination {
	const struct ieee80211_iface_limit *limits;
	int n_limits; int max_interfaces; int num_different_channels;
	bool beacon_int_infra_match; u8 radar_detect_widths;
};
struct station_del_parameters { const u8 *mac; u16 reason_code; };
#define IEEE80211_PRIVACY(privacy) ((privacy) ? IEEE80211_PRIVACY_ON : IEEE80211_PRIVACY_OFF)


struct bss_parameters;
struct cfg80211_ap_update;
struct cfg80211_assoc_request;
struct cfg80211_auth_request;
struct cfg80211_bitrate_mask;
struct cfg80211_bss_ies;
struct cfg80211_coalesce;
struct cfg80211_color_change_settings;
struct cfg80211_csa_settings;
struct cfg80211_deauth_request;
struct cfg80211_disassoc_request;
struct cfg80211_external_auth_params;
struct cfg80211_fils_aad;
struct cfg80211_ftm_responder_stats;
struct cfg80211_gtk_rekey_data;
struct cfg80211_ml_reconf_req;
struct cfg80211_nan_conf;
struct cfg80211_nan_func;
struct cfg80211_nan_local_sched;
struct cfg80211_nan_peer_sched;
struct cfg80211_ops;
struct cfg80211_pmk_conf;
struct cfg80211_pmsr_request;
struct cfg80211_qos_map;
struct cfg80211_sar_specs;
struct cfg80211_sched_scan_request;
struct cfg80211_set_hw_timestamp;
struct cfg80211_tid_config;
struct cfg80211_ttlm_params;
struct cfg80211_txq_stats;
struct cfg80211_update_ft_ies_params;
struct cfg80211_update_owe_info;
struct cfg80211_wowlan;
struct link_station_del_parameters;
struct link_station_parameters;
struct mesh_config;
struct mesh_setup;
struct mgmt_frame_regs;
struct mpath_info;
struct netlink_callback;
struct ocb_setup;
struct sk_buff;
struct station_del_parameters;
struct vif_params;
struct cfg80211_ops {
	int	(*suspend)(struct wiphy *wiphy, struct cfg80211_wowlan *wow);
	int	(*resume)(struct wiphy *wiphy);
	void	(*set_wakeup)(struct wiphy *wiphy, bool enabled);

	struct wireless_dev * (*add_virtual_intf)(struct wiphy *wiphy,
						  const char *name,
						  unsigned char name_assign_type,
						  enum nl80211_iftype type,
						  struct vif_params *params);
	int	(*del_virtual_intf)(struct wiphy *wiphy,
				    struct wireless_dev *wdev);
	int	(*change_virtual_intf)(struct wiphy *wiphy,
				       struct net_device *dev,
				       enum nl80211_iftype type,
				       struct vif_params *params);

	int	(*add_intf_link)(struct wiphy *wiphy,
				 struct wireless_dev *wdev,
				 unsigned int link_id);
	void	(*del_intf_link)(struct wiphy *wiphy,
				 struct wireless_dev *wdev,
				 unsigned int link_id);

	int	(*add_key)(struct wiphy *wiphy, struct wireless_dev *wdev,
			   int link_id, u8 key_index, bool pairwise,
			   const u8 *mac_addr, struct key_params *params);
	int	(*get_key)(struct wiphy *wiphy, struct wireless_dev *wdev,
			   int link_id, u8 key_index, bool pairwise,
			   const u8 *mac_addr, void *cookie,
			   void (*callback)(void *cookie, struct key_params*));
	int	(*del_key)(struct wiphy *wiphy, struct wireless_dev *wdev,
			   int link_id, u8 key_index, bool pairwise,
			   const u8 *mac_addr);
	int	(*set_default_key)(struct wiphy *wiphy,
				   struct net_device *netdev, int link_id,
				   u8 key_index, bool unicast, bool multicast);
	int	(*set_default_mgmt_key)(struct wiphy *wiphy,
					struct wireless_dev *wdev, int link_id,
					u8 key_index);
	int	(*set_default_beacon_key)(struct wiphy *wiphy,
					  struct wireless_dev *wdev,
					  int link_id,
					  u8 key_index);

	int	(*start_ap)(struct wiphy *wiphy, struct net_device *dev,
			    struct cfg80211_ap_settings *settings);
	int	(*change_beacon)(struct wiphy *wiphy, struct net_device *dev,
				 struct cfg80211_ap_update *info);
	int	(*stop_ap)(struct wiphy *wiphy, struct net_device *dev,
			   unsigned int link_id);


	int	(*add_station)(struct wiphy *wiphy, struct wireless_dev *wdev,
			       const u8 *mac,
			       struct station_parameters *params);
	int	(*del_station)(struct wiphy *wiphy, struct wireless_dev *wdev,
			       struct station_del_parameters *params);
	int	(*change_station)(struct wiphy *wiphy, struct wireless_dev *wdev,
				  const u8 *mac,
				  struct station_parameters *params);
	int	(*get_station)(struct wiphy *wiphy, struct wireless_dev *wdev,
			       const u8 *mac, struct station_info *sinfo);
	int	(*dump_station)(struct wiphy *wiphy, struct wireless_dev *wdev,
				int idx, u8 *mac, struct station_info *sinfo);

	int	(*add_mpath)(struct wiphy *wiphy, struct net_device *dev,
			       const u8 *dst, const u8 *next_hop);
	int	(*del_mpath)(struct wiphy *wiphy, struct net_device *dev,
			       const u8 *dst);
	int	(*change_mpath)(struct wiphy *wiphy, struct net_device *dev,
				  const u8 *dst, const u8 *next_hop);
	int	(*get_mpath)(struct wiphy *wiphy, struct net_device *dev,
			     u8 *dst, u8 *next_hop, struct mpath_info *pinfo);
	int	(*dump_mpath)(struct wiphy *wiphy, struct net_device *dev,
			      int idx, u8 *dst, u8 *next_hop,
			      struct mpath_info *pinfo);
	int	(*get_mpp)(struct wiphy *wiphy, struct net_device *dev,
			   u8 *dst, u8 *mpp, struct mpath_info *pinfo);
	int	(*dump_mpp)(struct wiphy *wiphy, struct net_device *dev,
			    int idx, u8 *dst, u8 *mpp,
			    struct mpath_info *pinfo);
	int	(*get_mesh_config)(struct wiphy *wiphy,
				struct net_device *dev,
				struct mesh_config *conf);
	int	(*update_mesh_config)(struct wiphy *wiphy,
				      struct net_device *dev, u32 mask,
				      const struct mesh_config *nconf);
	int	(*join_mesh)(struct wiphy *wiphy, struct net_device *dev,
			     const struct mesh_config *conf,
			     const struct mesh_setup *setup);
	int	(*leave_mesh)(struct wiphy *wiphy, struct net_device *dev);

	int	(*join_ocb)(struct wiphy *wiphy, struct net_device *dev,
			    struct ocb_setup *setup);
	int	(*leave_ocb)(struct wiphy *wiphy, struct net_device *dev);

	int	(*change_bss)(struct wiphy *wiphy, struct net_device *dev,
			      struct bss_parameters *params);

	void	(*inform_bss)(struct wiphy *wiphy, struct cfg80211_bss *bss,
			      const struct cfg80211_bss_ies *ies, void *data);

	int	(*set_txq_params)(struct wiphy *wiphy, struct net_device *dev,
				  struct ieee80211_txq_params *params);

	int	(*libertas_set_mesh_channel)(struct wiphy *wiphy,
					     struct net_device *dev,
					     struct ieee80211_channel *chan);

	int	(*set_monitor_channel)(struct wiphy *wiphy,
				       struct net_device *dev,
				       struct cfg80211_chan_def *chandef);

	int	(*scan)(struct wiphy *wiphy,
			struct cfg80211_scan_request *request);
	void	(*abort_scan)(struct wiphy *wiphy, struct wireless_dev *wdev);

	int	(*auth)(struct wiphy *wiphy, struct net_device *dev,
			struct cfg80211_auth_request *req);
	int	(*assoc)(struct wiphy *wiphy, struct net_device *dev,
			 struct cfg80211_assoc_request *req);
	int	(*deauth)(struct wiphy *wiphy, struct net_device *dev,
			  struct cfg80211_deauth_request *req);
	int	(*disassoc)(struct wiphy *wiphy, struct net_device *dev,
			    struct cfg80211_disassoc_request *req);

	int	(*connect)(struct wiphy *wiphy, struct net_device *dev,
			   struct cfg80211_connect_params *sme);
	int	(*update_connect_params)(struct wiphy *wiphy,
					 struct net_device *dev,
					 struct cfg80211_connect_params *sme,
					 u32 changed);
	int	(*disconnect)(struct wiphy *wiphy, struct net_device *dev,
			      u16 reason_code);

	int	(*join_ibss)(struct wiphy *wiphy, struct net_device *dev,
			     struct cfg80211_ibss_params *params);
	int	(*leave_ibss)(struct wiphy *wiphy, struct net_device *dev);

	int	(*set_mcast_rate)(struct wiphy *wiphy, struct net_device *dev,
				  int rate[NUM_NL80211_BANDS]);

	int	(*set_wiphy_params)(struct wiphy *wiphy, int radio_idx,
				    u32 changed);

	int	(*set_tx_power)(struct wiphy *wiphy, struct wireless_dev *wdev,
				int radio_idx,
				enum nl80211_tx_power_setting type, int mbm);
	int	(*get_tx_power)(struct wiphy *wiphy, struct wireless_dev *wdev,
				int radio_idx, unsigned int link_id, int *dbm);

	void	(*rfkill_poll)(struct wiphy *wiphy);

#ifdef CONFIG_NL80211_TESTMODE
	int	(*testmode_cmd)(struct wiphy *wiphy, struct wireless_dev *wdev,
				void *data, int len);
	int	(*testmode_dump)(struct wiphy *wiphy, struct sk_buff *skb,
				 struct netlink_callback *cb,
				 void *data, int len);
#endif

	int	(*set_bitrate_mask)(struct wiphy *wiphy,
				    struct net_device *dev,
				    unsigned int link_id,
				    const u8 *peer,
				    const struct cfg80211_bitrate_mask *mask);

	int	(*dump_survey)(struct wiphy *wiphy, struct net_device *netdev,
			int idx, struct survey_info *info);

	int	(*set_pmksa)(struct wiphy *wiphy, struct net_device *netdev,
			     struct cfg80211_pmksa *pmksa);
	int	(*del_pmksa)(struct wiphy *wiphy, struct net_device *netdev,
			     struct cfg80211_pmksa *pmksa);
	int	(*flush_pmksa)(struct wiphy *wiphy, struct net_device *netdev);

	int	(*remain_on_channel)(struct wiphy *wiphy,
				     struct wireless_dev *wdev,
				     struct ieee80211_channel *chan,
				     unsigned int duration,
				     u64 *cookie);
	int	(*cancel_remain_on_channel)(struct wiphy *wiphy,
					    struct wireless_dev *wdev,
					    u64 cookie);

	int	(*mgmt_tx)(struct wiphy *wiphy, struct wireless_dev *wdev,
			   struct cfg80211_mgmt_tx_params *params,
			   u64 *cookie);
	int	(*mgmt_tx_cancel_wait)(struct wiphy *wiphy,
				       struct wireless_dev *wdev,
				       u64 cookie);

	int	(*set_power_mgmt)(struct wiphy *wiphy, struct net_device *dev,
				  bool enabled, int timeout);

	int	(*set_cqm_rssi_config)(struct wiphy *wiphy,
				       struct net_device *dev,
				       s32 rssi_thold, u32 rssi_hyst);

	int	(*set_cqm_rssi_range_config)(struct wiphy *wiphy,
					     struct net_device *dev,
					     s32 rssi_low, s32 rssi_high);

	int	(*set_cqm_txe_config)(struct wiphy *wiphy,
				      struct net_device *dev,
				      u32 rate, u32 pkts, u32 intvl);

	void	(*update_mgmt_frame_registrations)(struct wiphy *wiphy,
						   struct wireless_dev *wdev,
						   struct mgmt_frame_regs *upd);

	int	(*set_antenna)(struct wiphy *wiphy, int radio_idx,
			       u32 tx_ant, u32 rx_ant);
	int	(*get_antenna)(struct wiphy *wiphy, int radio_idx,
			       u32 *tx_ant, u32 *rx_ant);

	int	(*sched_scan_start)(struct wiphy *wiphy,
				struct net_device *dev,
				struct cfg80211_sched_scan_request *request);
	int	(*sched_scan_stop)(struct wiphy *wiphy, struct net_device *dev,
				   u64 reqid);

	int	(*set_rekey_data)(struct wiphy *wiphy, struct net_device *dev,
				  struct cfg80211_gtk_rekey_data *data);

	int	(*tdls_mgmt)(struct wiphy *wiphy, struct net_device *dev,
			     const u8 *peer, int link_id,
			     u8 action_code, u8 dialog_token, u16 status_code,
			     u32 peer_capability, bool initiator,
			     const u8 *buf, size_t len);
	int	(*tdls_oper)(struct wiphy *wiphy, struct net_device *dev,
			     const u8 *peer, enum nl80211_tdls_operation oper);

	int	(*probe_client)(struct wiphy *wiphy, struct net_device *dev,
				const u8 *peer, u64 *cookie);

	int	(*set_noack_map)(struct wiphy *wiphy,
				  struct net_device *dev,
				  u16 noack_map);

	int	(*get_channel)(struct wiphy *wiphy,
			       struct wireless_dev *wdev,
			       unsigned int link_id,
			       struct cfg80211_chan_def *chandef);

	int	(*start_p2p_device)(struct wiphy *wiphy,
				    struct wireless_dev *wdev);
	void	(*stop_p2p_device)(struct wiphy *wiphy,
				   struct wireless_dev *wdev);

	int	(*set_mac_acl)(struct wiphy *wiphy, struct net_device *dev,
			       const struct cfg80211_acl_data *params);

	int	(*start_radar_detection)(struct wiphy *wiphy,
					 struct net_device *dev,
					 struct cfg80211_chan_def *chandef,
					 u32 cac_time_ms, int link_id);
	void	(*end_cac)(struct wiphy *wiphy,
			   struct net_device *dev, unsigned int link_id);
	int	(*update_ft_ies)(struct wiphy *wiphy, struct net_device *dev,
				 struct cfg80211_update_ft_ies_params *ftie);
	int	(*crit_proto_start)(struct wiphy *wiphy,
				    struct wireless_dev *wdev,
				    enum nl80211_crit_proto_id protocol,
				    u16 duration);
	void	(*crit_proto_stop)(struct wiphy *wiphy,
				   struct wireless_dev *wdev);
	int	(*set_coalesce)(struct wiphy *wiphy,
				struct cfg80211_coalesce *coalesce);

	int	(*channel_switch)(struct wiphy *wiphy,
				  struct net_device *dev,
				  struct cfg80211_csa_settings *params);

	int     (*set_qos_map)(struct wiphy *wiphy,
			       struct net_device *dev,
			       struct cfg80211_qos_map *qos_map);

	int	(*set_ap_chanwidth)(struct wiphy *wiphy, struct net_device *dev,
				    unsigned int link_id,
				    struct cfg80211_chan_def *chandef);

	int	(*add_tx_ts)(struct wiphy *wiphy, struct net_device *dev,
			     u8 tsid, const u8 *peer, u8 user_prio,
			     u16 admitted_time);
	int	(*del_tx_ts)(struct wiphy *wiphy, struct net_device *dev,
			     u8 tsid, const u8 *peer);

	int	(*tdls_channel_switch)(struct wiphy *wiphy,
				       struct net_device *dev,
				       const u8 *addr, u8 oper_class,
				       struct cfg80211_chan_def *chandef);
	void	(*tdls_cancel_channel_switch)(struct wiphy *wiphy,
					      struct net_device *dev,
					      const u8 *addr);
	int	(*start_nan)(struct wiphy *wiphy, struct wireless_dev *wdev,
			     struct cfg80211_nan_conf *conf);
	void	(*stop_nan)(struct wiphy *wiphy, struct wireless_dev *wdev);
	int	(*add_nan_func)(struct wiphy *wiphy, struct wireless_dev *wdev,
				struct cfg80211_nan_func *nan_func);
	void	(*del_nan_func)(struct wiphy *wiphy, struct wireless_dev *wdev,
			       u64 cookie);
	int	(*nan_change_conf)(struct wiphy *wiphy,
				   struct wireless_dev *wdev,
				   struct cfg80211_nan_conf *conf,
				   u32 changes);
	int	(*nan_set_local_sched)(struct wiphy *wiphy,
				       struct wireless_dev *wdev,
				       struct cfg80211_nan_local_sched *sched);
	int	(*nan_set_peer_sched)(struct wiphy *wiphy,
				      struct wireless_dev *wdev,
				      struct cfg80211_nan_peer_sched *sched);
	int	(*set_multicast_to_unicast)(struct wiphy *wiphy,
					    struct net_device *dev,
					    const bool enabled);

	int	(*get_txq_stats)(struct wiphy *wiphy,
				 struct wireless_dev *wdev,
				 struct cfg80211_txq_stats *txqstats);

	int	(*set_pmk)(struct wiphy *wiphy, struct net_device *dev,
			   const struct cfg80211_pmk_conf *conf);
	int	(*del_pmk)(struct wiphy *wiphy, struct net_device *dev,
			   const u8 *aa);
	int     (*external_auth)(struct wiphy *wiphy, struct net_device *dev,
				 struct cfg80211_external_auth_params *params);

	int	(*tx_control_port)(struct wiphy *wiphy,
				   struct net_device *dev,
				   const u8 *buf, size_t len,
				   const u8 *dest, const __be16 proto,
				   const bool noencrypt, int link_id,
				   u64 *cookie);

	int	(*get_ftm_responder_stats)(struct wiphy *wiphy,
				struct net_device *dev,
				struct cfg80211_ftm_responder_stats *ftm_stats);

	int	(*start_pmsr)(struct wiphy *wiphy, struct wireless_dev *wdev,
			      struct cfg80211_pmsr_request *request);
	void	(*abort_pmsr)(struct wiphy *wiphy, struct wireless_dev *wdev,
			      struct cfg80211_pmsr_request *request);
	int	(*update_owe_info)(struct wiphy *wiphy, struct net_device *dev,
				   struct cfg80211_update_owe_info *owe_info);
	int	(*probe_mesh_link)(struct wiphy *wiphy, struct net_device *dev,
				   const u8 *buf, size_t len);
	int     (*set_tid_config)(struct wiphy *wiphy, struct net_device *dev,
				  struct cfg80211_tid_config *tid_conf);
	int	(*reset_tid_config)(struct wiphy *wiphy, struct net_device *dev,
				    const u8 *peer, u8 tids);
	int	(*set_sar_specs)(struct wiphy *wiphy,
				 struct cfg80211_sar_specs *sar);
	int	(*color_change)(struct wiphy *wiphy,
				struct net_device *dev,
				struct cfg80211_color_change_settings *params);
	int     (*set_fils_aad)(struct wiphy *wiphy, struct net_device *dev,
				struct cfg80211_fils_aad *fils_aad);
	int	(*set_radar_background)(struct wiphy *wiphy,
					struct cfg80211_chan_def *chandef);
	int	(*add_link_station)(struct wiphy *wiphy, struct net_device *dev,
				    struct link_station_parameters *params);
	int	(*mod_link_station)(struct wiphy *wiphy, struct net_device *dev,
				    struct link_station_parameters *params);
	int	(*del_link_station)(struct wiphy *wiphy, struct net_device *dev,
				    struct link_station_del_parameters *params);
	int	(*set_hw_timestamp)(struct wiphy *wiphy, struct net_device *dev,
				    struct cfg80211_set_hw_timestamp *hwts);
	int	(*set_ttlm)(struct wiphy *wiphy, struct net_device *dev,
			    struct cfg80211_ttlm_params *params);
	u32	(*get_radio_mask)(struct wiphy *wiphy, struct net_device *dev);
	int     (*assoc_ml_reconf)(struct wiphy *wiphy, struct net_device *dev,
				   struct cfg80211_ml_reconf_req *req);
	int	(*set_epcs)(struct wiphy *wiphy, struct net_device *dev,
			    bool val);
};

/* ===== API ===== */
struct wiphy *wiphy_new(const struct cfg80211_ops *ops, int sizeof_priv);
int   wiphy_register(struct wiphy *wiphy);
void  wiphy_unregister(struct wiphy *wiphy);
void  wiphy_free(struct wiphy *wiphy);
void  wiphy_apply_custom_regulatory(struct wiphy *wiphy, const struct ieee80211_regdomain *regd);
int   regulatory_hint(struct wiphy *wiphy, const char *alpha2);
void  wiphy_rfkill_set_hw_state(struct wiphy *wiphy, bool blocked);
static inline void wiphy_rfkill_start_polling(struct wiphy *w) { (void)w; }
static inline void wiphy_rfkill_stop_polling(struct wiphy *w) { (void)w; }

struct ieee80211_channel *ieee80211_get_channel(struct wiphy *wiphy, int freq);

/* a driver ezeket hívja a scan/bss/kapcsolat eredmények bejelentésére */
void cfg80211_scan_done(struct cfg80211_scan_request *request, struct cfg80211_scan_info *info);
struct cfg80211_bss *cfg80211_inform_bss(struct wiphy *wiphy, struct ieee80211_channel *chan,
		const u8 *bssid, u64 tsf, u16 cap, u16 beacon_int,
		const u8 *ie, size_t ielen, s32 signal, gfp_t gfp);
struct cfg80211_bss *cfg80211_get_bss(struct wiphy *wiphy, struct ieee80211_channel *chan,
		const u8 *bssid, const u8 *ssid, size_t ssid_len, u32 _u1, u32 _u2);
void cfg80211_put_bss(struct wiphy *wiphy, struct cfg80211_bss *bss);
void cfg80211_unlink_bss(struct wiphy *wiphy, struct cfg80211_bss *bss);

/* event-bejelentések (stub-szintű deklarációk; a cfg80211.so implementálja) */
void cfg80211_connect_result(struct net_device *dev, const u8 *bssid, const u8 *req_ie, size_t req_ie_len,
		const u8 *resp_ie, size_t resp_ie_len, u16 status, gfp_t gfp);
struct cfg80211_roam_info {
	struct ieee80211_channel *channel;
	struct cfg80211_bss *bss;
	const u8 *bssid;
	const u8 *req_ie; size_t req_ie_len;
	const u8 *resp_ie; size_t resp_ie_len;
};
void cfg80211_roamed(struct net_device *dev, struct cfg80211_roam_info *info, gfp_t gfp);
void cfg80211_connect_bss(struct net_device *dev, const u8 *bssid, struct cfg80211_bss *bss,
		const u8 *req_ie, size_t req_ie_len, const u8 *resp_ie, size_t resp_ie_len,
		int status, gfp_t gfp, int timeout_reason);
void cfg80211_disconnected(struct net_device *dev, u16 reason, const u8 *ie, size_t ie_len, bool locally, gfp_t gfp);
void cfg80211_ibss_joined(struct net_device *dev, const u8 *bssid, struct ieee80211_channel *chan, gfp_t gfp);
void cfg80211_new_sta(struct net_device *dev, const u8 *mac, struct station_info *sinfo, gfp_t gfp);
void cfg80211_del_sta(struct net_device *dev, const u8 *mac, gfp_t gfp);
void cfg80211_rx_mgmt(struct wireless_dev *wdev, int freq, int sig, const u8 *buf, size_t len, u32 flags);
void cfg80211_mgmt_tx_status(struct wireless_dev *wdev, u64 cookie, const u8 *buf, size_t len, bool ack, gfp_t gfp);
void cfg80211_ready_on_channel(struct wireless_dev *wdev, u64 cookie, struct ieee80211_channel *chan,
		unsigned int duration, gfp_t gfp);
void cfg80211_remain_on_channel_expired(struct wireless_dev *wdev, u64 cookie, struct ieee80211_channel *chan, gfp_t gfp);
void cfg80211_michael_mic_failure(struct net_device *dev, const u8 *addr, int key_type, int key_id, const u8 *tsc, gfp_t gfp);
void cfg80211_cqm_rssi_notify(struct net_device *dev, int event, s32 level, gfp_t gfp);
void cfg80211_ch_switch_notify(struct net_device *dev, struct cfg80211_chan_def *chandef, unsigned int link_id, u16 punct_bitmap);
void cfg80211_sched_scan_results(struct wiphy *wiphy, u64 reqid);
void cfg80211_unregister_wdev(struct wireless_dev *wdev);
int  cfg80211_sinfo_alloc_tid_stats(struct station_info *sinfo, gfp_t gfp);
struct ieee80211_channel *ieee80211_get_channel_khz(struct wiphy *wiphy, u32 freq);


void cfg80211_ch_switch_started_notify(struct net_device *dev, struct cfg80211_chan_def *chandef,
		unsigned int link_id, u8 count, bool quiet, u16 punct_bitmap);
struct cfg80211_bss *cfg80211_inform_bss_frame(struct wiphy *wiphy, struct ieee80211_channel *chan,
		struct ieee80211_mgmt *mgmt, size_t len, s32 signal, gfp_t gfp);
void cfg80211_send_rx_assoc(struct net_device *dev, struct cfg80211_bss *bss, const u8 *buf, size_t len);
void cfg80211_send_disassoc(struct net_device *dev, const u8 *buf, size_t len);
void cfg80211_external_auth_request(struct net_device *dev, void *params, gfp_t gfp);

#endif /* _UK_NET_CFG80211_H */

/* uKernel hamis <net/regulatory.h> — minimál szabályozási struktúrák. */
#ifndef _UK_NET_REGULATORY_H
#define _UK_NET_REGULATORY_H
#define REGULATORY_CUSTOM_REG            (1<<0)
#define REGULATORY_STRICT_REG           (1<<1)
#define REGULATORY_DISABLE_BEACON_HINTS (1<<2)
#define REGULATORY_COUNTRY_IE_IGNORE    (1<<7)

#include <linux/types.h>

struct ieee80211_freq_range { u32 start_freq_khz, end_freq_khz, max_bandwidth_khz; };
struct ieee80211_power_rule  { u32 max_antenna_gain, max_eirp; };
struct ieee80211_reg_rule {
	struct ieee80211_freq_range freq_range;
	struct ieee80211_power_rule power_rule;
	u32 flags;
};
struct ieee80211_regdomain {
	u32   n_reg_rules;
	char  alpha2[2];
	struct ieee80211_reg_rule reg_rules[];
};

#define REG_RULE(start, end, bw, gain, eirp, reg_flags) \
	{ .freq_range = { (start) * 1000, (end) * 1000, (bw) * 1000 }, \
	  .power_rule = { (gain), (eirp) }, .flags = (reg_flags) }

#endif

/* uKernel hamis <net/mac80211.h> — VÉKONY STUB.
 *
 * A rtl8812au full-MAC (cfg80211-t használ közvetlenül), ezért a mac80211-re
 * NINCS szüksége. Ez a fejléc+stub azért van, hogy egy soft-MAC driver (ami
 * mac80211-re épül) is betölthető legyen a uServerbe. A tényleges mac80211
 * funkció (rate control, aggregáció, TX/RX path) későbbi bővítés. */
#ifndef _UK_NET_MAC80211_H
#define _UK_NET_MAC80211_H

#include <linux/types.h>
#include <linux/skbuff.h>
#include <net/cfg80211.h>

struct ieee80211_hw {
	struct wiphy *wiphy;
	void  *priv;
	u32    flags;
	int    queues;
};

struct ieee80211_ops {
	void (*tx)(struct ieee80211_hw *hw, void *control, struct sk_buff *skb);
	int  (*start)(struct ieee80211_hw *hw);
	void (*stop)(struct ieee80211_hw *hw);
	int  (*add_interface)(struct ieee80211_hw *hw, void *vif);
	int  (*config)(struct ieee80211_hw *hw, u32 changed);
};

struct ieee80211_hw *ieee80211_alloc_hw(size_t priv_data_len, const struct ieee80211_ops *ops);
int  ieee80211_register_hw(struct ieee80211_hw *hw);
void ieee80211_unregister_hw(struct ieee80211_hw *hw);
void ieee80211_free_hw(struct ieee80211_hw *hw);
void ieee80211_rx(struct ieee80211_hw *hw, struct sk_buff *skb);
void ieee80211_tx_status(struct ieee80211_hw *hw, struct sk_buff *skb);
static inline struct wiphy *ieee80211_hw_to_wiphy(struct ieee80211_hw *hw) { return hw->wiphy; }

#endif

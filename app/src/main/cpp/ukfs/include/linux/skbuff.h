/* uKernel hamis <linux/skbuff.h> — minimál sk_buff a hálózati/wifi úthoz. */
#ifndef _UK_LINUX_SKBUFF_H
#define _UK_LINUX_SKBUFF_H

#include <linux/types.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <string.h>

struct net_device;   /* forward — a netdev_alloc_skb-hez (a netdevice.h minket include-ol) */
struct napi_struct;

struct net_device;
struct sk_buff;

struct skb_frag { void *page; unsigned int off; unsigned int size; };
struct skb_shared_info {
	unsigned short	nr_frags;
	unsigned short	gso_size;
	unsigned short	gso_segs;
	unsigned int	gso_type;
	struct sk_buff	*frag_list;
	struct skb_frag frags[17];
};

struct sk_buff {
	struct sk_buff *next, *prev;
	struct net_device *dev_real;
	unsigned char  *head, *data, *tail, *end;
	unsigned int    len;
	unsigned int    data_len;       /* paged data length */
	unsigned int    truesize;
	void           *dev;
	struct sk_buff *sg;             /* usbnet can_dma_sg fields (egyszerusitett) */
	unsigned int    num_sgs;
	u16             protocol;
	u32             priority;
	u32             mark;
	u16             queue_mapping;
	u8              ip_summed;
	u8              pkt_type;
	u8              cloned;
	__wsum          csum;
	__u32           csum_start;
	__u32           csum_offset;
	unsigned char   cb[48];         /* control block — driverek skb->cb-t használnak */
	struct skb_shared_info shinfo;
};

#define SINGLE_DEPTH_NESTING 1
#define MAX_SKB_FRAGS 17
typedef struct skb_frag skb_frag_t;

static inline struct skb_shared_info *skb_shinfo(const struct sk_buff *skb)
{ return (struct skb_shared_info *)&((struct sk_buff *)skb)->shinfo; }

#define CHECKSUM_NONE     0
#define CHECKSUM_UNNECESSARY 1
#define CHECKSUM_COMPLETE 2
#define CHECKSUM_PARTIAL  3
#define PACKET_HOST       0
#define PACKET_BROADCAST  1
#define PACKET_MULTICAST  2
#define PACKET_OTHERHOST  3
#define NET_RX_SUCCESS    0
#define NET_RX_DROP       1
#ifndef NET_IP_ALIGN
#define NET_IP_ALIGN      2
#endif

struct sk_buff_head {
	struct sk_buff *next, *prev;
	unsigned int    qlen;
	spinlock_t      lock;
};

struct sk_buff *alloc_skb(unsigned int size, gfp_t gfp);
/* mint a valódi kernel: a dev_alloc_skb NET_SKB_PAD headroom-ot foglal — kell a
 * monitor-RX radiotap-fejléc skb_push-ához (fill_radiotap_hdr) */
#define NET_SKB_PAD 128
static inline struct sk_buff *__dev_alloc_skb(unsigned int size, gfp_t gfp)
{
	struct sk_buff *skb = alloc_skb(size + NET_SKB_PAD, gfp);
	if (skb) { skb->data += NET_SKB_PAD; skb->tail += NET_SKB_PAD; }
	return skb;
}
static inline struct sk_buff *dev_alloc_skb(unsigned int size) { return __dev_alloc_skb(size, GFP_ATOMIC); }
struct sk_buff *skb_clone(struct sk_buff *skb, gfp_t gfp);
struct sk_buff *skb_copy(const struct sk_buff *skb, gfp_t gfp);
static inline struct sk_buff *pskb_copy(struct sk_buff *skb, gfp_t gfp) { return skb_copy(skb, gfp); }
struct sk_buff *netdev_alloc_skb(struct net_device *dev, unsigned int len);
struct sk_buff *__netdev_alloc_skb(struct net_device *dev, unsigned int len, gfp_t gfp);
struct sk_buff *__netdev_alloc_skb_ip_align(struct net_device *dev, unsigned int len, gfp_t gfp);
static inline struct sk_buff *netdev_alloc_skb_ip_align(struct net_device *dev, unsigned int len)
{ return __netdev_alloc_skb_ip_align(dev, len, GFP_ATOMIC); }
struct sk_buff *napi_alloc_skb(struct napi_struct *napi, unsigned int len);
void kfree_skb(struct sk_buff *skb);
static inline void dev_kfree_skb(struct sk_buff *skb) { kfree_skb(skb); }
static inline void dev_kfree_skb_any(struct sk_buff *skb) { kfree_skb(skb); }

void *skb_put(struct sk_buff *skb, unsigned int len);
void *skb_push(struct sk_buff *skb, unsigned int len);
void *skb_pull(struct sk_buff *skb, unsigned int len);
void skb_reserve(struct sk_buff *skb, int len);
void skb_trim(struct sk_buff *skb, unsigned int len);
static inline unsigned int skb_headroom(const struct sk_buff *skb) { return skb->data - skb->head; }
static inline unsigned int skb_tailroom(const struct sk_buff *skb) { return skb->end - skb->tail; }
static inline unsigned char *skb_tail_pointer(const struct sk_buff *skb) { return skb->tail; }
static inline unsigned char *skb_end_pointer(const struct sk_buff *skb) { return skb->end; }
static inline void skb_set_tail_pointer(struct sk_buff *skb, int off) { skb->tail = skb->data + off; }
static inline void skb_reset_tail_pointer(struct sk_buff *skb) { skb->tail = skb->data; }
static inline void skb_reset_mac_header(struct sk_buff *skb) { (void)skb; }
static inline void skb_reset_network_header(struct sk_buff *skb) { (void)skb; }
static inline unsigned char *skb_mac_header(const struct sk_buff *skb) { return skb->data; }
static inline void skb_set_network_header(struct sk_buff *skb, int off) { (void)skb; (void)off; }
static inline void skb_set_queue_mapping(struct sk_buff *skb, u16 q) { skb->queue_mapping = q; }
static inline u16  skb_get_queue_mapping(const struct sk_buff *skb) { return skb->queue_mapping; }
int skb_copy_bits(const struct sk_buff *skb, int offset, void *to, int len);

void skb_queue_head_init(struct sk_buff_head *list);
void skb_queue_tail(struct sk_buff_head *list, struct sk_buff *newsk);
struct sk_buff *skb_dequeue(struct sk_buff_head *list);
static inline int skb_queue_empty(const struct sk_buff_head *list) { return list->next == (const struct sk_buff *)list; }
static inline u32 skb_queue_len(const struct sk_buff_head *list) { return list->qlen; }

/* ===== usbnet / net-usb kiegeszitesek ===== */

#define skb_queue_walk(queue, skb) \
	for ((skb) = (queue)->next; \
	     (skb) != (struct sk_buff *)(queue); \
	     (skb) = (skb)->next)

#define skb_queue_walk_safe(queue, skb, tmp) \
	for ((skb) = (queue)->next, (tmp) = (skb)->next; \
	     (skb) != (struct sk_buff *)(queue); \
	     (skb) = (tmp), (tmp) = (skb)->next)

#define skb_list_walk_safe(first, skb, next_skb) \
	for ((skb) = (first), (next_skb) = (skb) ? (skb)->next : NULL; (skb); \
	     (skb) = (next_skb), (next_skb) = (skb) ? (skb)->next : NULL)

static inline void skb_mark_not_on_list(struct sk_buff *skb) { skb->next = NULL; }

static inline unsigned int skb_frag_size(const skb_frag_t *frag) { return frag->size; }
static inline void *skb_frag_page(const skb_frag_t *frag) { return frag->page; }
static inline unsigned int skb_frag_off(const skb_frag_t *frag) { return frag->off; }
void skb_add_rx_frag(struct sk_buff *skb, int i, void *page, int off,
		     int size, unsigned int truesize);

static inline unsigned int skb_headlen(const struct sk_buff *skb)
{ return skb->len - skb->data_len; }
static inline int skb_is_nonlinear(const struct sk_buff *skb)
{ return skb->data_len != 0; }
static inline int skb_cloned(const struct sk_buff *skb) { return skb->cloned; }
static inline int skb_header_cloned(const struct sk_buff *skb) { return skb->cloned; }
static inline int skb_is_gso(const struct sk_buff *skb) { return skb_shinfo(skb)->gso_size; }

void __skb_queue_head_init(struct sk_buff_head *list);
void __skb_queue_tail(struct sk_buff_head *list, struct sk_buff *newsk);
void __skb_queue_head(struct sk_buff_head *list, struct sk_buff *newsk);
struct sk_buff *__skb_dequeue(struct sk_buff_head *list);
void __skb_unlink(struct sk_buff *skb, struct sk_buff_head *list);
void skb_queue_purge(struct sk_buff_head *list);
void skb_queue_splice(const struct sk_buff_head *list, struct sk_buff_head *head);
void skb_queue_splice_init(struct sk_buff_head *list, struct sk_buff_head *head);
struct sk_buff *skb_peek(const struct sk_buff_head *list);

void *__skb_put(struct sk_buff *skb, unsigned int len);
void *__skb_push(struct sk_buff *skb, unsigned int len);
void *__skb_pull(struct sk_buff *skb, unsigned int len);
static inline void *skb_put_data(struct sk_buff *skb, const void *data, unsigned int len)
{ void *tmp = skb_put(skb, len); memcpy(tmp, data, len); return tmp; }
static inline void skb_put_u8(struct sk_buff *skb, u8 val)
{ *(u8 *)skb_put(skb, 1) = val; }
static inline void *skb_put_zero(struct sk_buff *skb, unsigned int len)
{ void *tmp = skb_put(skb, len); memset(tmp, 0, len); return tmp; }
static inline void *__skb_put_zero(struct sk_buff *skb, unsigned int len)
{ void *tmp = __skb_put(skb, len); memset(tmp, 0, len); return tmp; }

static inline void skb_copy_to_linear_data(struct sk_buff *skb, const void *from, unsigned int len)
{ memcpy(skb->data, from, len); }
static inline void skb_copy_to_linear_data_offset(struct sk_buff *skb, int off, const void *from, unsigned int len)
{ memcpy(skb->data + off, from, len); }
static inline void skb_copy_from_linear_data(const struct sk_buff *skb, void *to, unsigned int len)
{ memcpy(to, skb->data, len); }
static inline void skb_copy_from_linear_data_offset(const struct sk_buff *skb, int off, void *to, unsigned int len)
{ memcpy(to, skb->data + off, len); }

struct sk_buff *skb_copy_expand(const struct sk_buff *skb, int newheadroom,
				int newtailroom, gfp_t gfp);
int  skb_linearize(struct sk_buff *skb);
int  pskb_expand_head(struct sk_buff *skb, int nhead, int ntail, gfp_t gfp);
struct sk_buff *skb_expand_head(struct sk_buff *skb, unsigned int headroom);
int  __pskb_pull_tail(struct sk_buff *skb, int delta);
static inline int skb_padto(struct sk_buff *skb, unsigned int len) { (void)skb; (void)len; return 0; }
int  skb_pad(struct sk_buff *skb, int pad);
int  skb_cow_head(struct sk_buff *skb, unsigned int headroom);
void consume_skb(struct sk_buff *skb);
static inline void dev_kfree_skb_irq(struct sk_buff *skb) { kfree_skb(skb); }
struct sk_buff *skb_get(struct sk_buff *skb);

static inline int skb_transport_offset(const struct sk_buff *skb) { (void)skb; return 0; }
static inline void skb_reset_transport_header(struct sk_buff *skb) { (void)skb; }
static inline void skb_set_transport_header(struct sk_buff *skb, int off) { (void)skb; (void)off; }
static inline unsigned char *skb_transport_header(const struct sk_buff *skb) { return skb->data; }
static inline unsigned char *skb_network_header(const struct sk_buff *skb) { return skb->data; }
static inline int skb_checksum_start_offset(const struct sk_buff *skb) { return skb->csum_start; }
int  skb_checksum_help(struct sk_buff *skb);
struct sk_buff *skb_gso_segment(struct sk_buff *skb, u64 features);
static inline unsigned int skb_gso_transport_seglen(const struct sk_buff *skb) { (void)skb; return 0; }
static inline void skb_tx_timestamp(struct sk_buff *skb) { (void)skb; }
static inline int skb_defer_rx_timestamp(struct sk_buff *skb) { (void)skb; return 0; }

/* napi (poll) — no-op a felhasznalonki I/O modellben */
struct napi_struct { int dummy; };
static inline void napi_schedule(struct napi_struct *n) { (void)n; }
static inline void napi_enable(struct napi_struct *n) { (void)n; }
static inline void napi_disable(struct napi_struct *n) { (void)n; }
static inline int  napi_complete_done(struct napi_struct *n, int w) { (void)n; (void)w; return 1; }
int napi_gro_receive(struct napi_struct *napi, struct sk_buff *skb);
struct sk_buff *napi_get_frags(struct napi_struct *napi);
int napi_gro_frags(struct napi_struct *napi);
static inline void napi_add(struct net_device *dev, struct napi_struct *napi, int (*poll)(struct napi_struct *, int)) { (void)dev;(void)napi;(void)poll; }

#endif

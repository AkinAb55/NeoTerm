#ifndef _UK_LINUX_IF_ETHER_H
#define _UK_LINUX_IF_ETHER_H
#include <linux/types.h>
#define ETH_ALEN   6
#define ETH_HLEN   14
#define ETH_P_IP   0x0800
#define ETH_P_ARP  0x0806
#define ETH_P_IPV6 0x86DD
#define ETH_P_PAE  0x888E
#define ETH_P_AARP 0x80F3
#define ETH_P_IPX  0x8137
#define ETH_P_RARP 0x8035
#define ETH_P_80211_RAW 0x0019
#define ETH_P_8021Q  0x8100
#define ETH_P_8021AD 0x88A8
#define ETH_P_802_3  0x0001
#define ETH_P_802_2  0x0004
#define ETH_P_ALL    0x0003
#define ETH_P_TEB    0x6558
#define ETH_P_MAP    0xDA1A
#define ETH_TLEN     2
struct ethhdr { u8 h_dest[6]; u8 h_source[6]; __be16 h_proto; } __attribute__((packed));
/* konstans (case label-ben is használható) byteswap */
#define ___swab16c(x) ((__u16)((((__u16)(x) & 0x00ff) << 8) | (((__u16)(x) & 0xff00) >> 8)))
/* erőltetjük a konstans verziót (case label-ben is jó); a glibc -O2-es htons-a
 * __builtin_constant_p-ternáriust használ, ami nem érvényes case-konstans */
#undef htons
#undef ntohs
#undef htonl
#undef ntohl
#define htons(x) ___swab16c(x)
#define ntohs(x) ___swab16c(x)
#define ___swab32c(x) ((__u32)((((__u32)(x)&0xff)<<24)|(((__u32)(x)&0xff00)<<8)|(((__u32)(x)>>8)&0xff00)|(((__u32)(x)>>24)&0xff)))
#define htonl(x) ___swab32c(x)
#define ntohl(x) ___swab32c(x)
#endif

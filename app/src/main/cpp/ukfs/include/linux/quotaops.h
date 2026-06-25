#ifndef _UK_LINUX_QUOTAOPS_H
#define _UK_LINUX_QUOTAOPS_H
#include <linux/quota.h>
struct inode;
static inline int dquot_initialize(struct inode *inode) { (void)inode; return 0; }
static inline void dquot_drop(struct inode *inode) { (void)inode; }
/* dquot_alloc_block: az i_blocks-frissítés az inode_add_bytes-on át (vfs.c, teljes struct inode) */
int uk_dquot_alloc_block(struct inode *inode, qsize_t nr);
static inline int dquot_alloc_block(struct inode *inode, qsize_t nr) { return uk_dquot_alloc_block(inode, nr); }
void uk_dquot_free_block(struct inode *inode, qsize_t nr);   /* i_blocks csökkentés (vfs.c) */
static inline void dquot_free_block(struct inode *inode, qsize_t nr) { uk_dquot_free_block(inode, nr); }
static inline int dquot_alloc_inode(struct inode *inode) { (void)inode; return 0; }
static inline void dquot_free_inode(struct inode *inode) { (void)inode; }
static inline int dquot_transfer(struct mnt_idmap *idmap, struct inode *inode, struct iattr *iattr) { (void)idmap;(void)inode;(void)iattr; return 0; }
static inline qsize_t dquot_alloc_block_nofail(struct inode *inode, qsize_t nr) { (void)inode; return nr; }
static inline int dquot_claim_block(struct inode *inode, qsize_t nr) { (void)inode;(void)nr; return 0; }
static inline void dquot_release_reservation_block(struct inode *inode, qsize_t nr) { (void)inode;(void)nr; }
static inline int dquot_reserve_block(struct inode *inode, qsize_t nr) { (void)inode;(void)nr; return 0; }
#endif

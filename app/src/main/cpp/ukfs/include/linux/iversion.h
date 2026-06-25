/* uKernel hamis <linux/iversion.h> — inode-verzió (i_version) kezelése. */
#ifndef _UK_LINUX_IVERSION_H
#define _UK_LINUX_IVERSION_H
#include <linux/fs.h>

static inline void inode_set_iversion(struct inode *inode, u64 val) { inode->i_version = val; }
static inline void inode_set_iversion_raw(struct inode *inode, u64 val) { inode->i_version = val; }
static inline void inode_set_iversion_queried(struct inode *inode, u64 val) { inode->i_version = val; }
static inline u64 inode_query_iversion(struct inode *inode) { return inode->i_version; }
static inline u64 inode_peek_iversion(const struct inode *inode) { return inode->i_version; }
static inline u64 inode_peek_iversion_raw(const struct inode *inode) { return inode->i_version; }
static inline bool inode_maybe_inc_iversion(struct inode *inode, bool force) { (void)force; inode->i_version++; return true; }
static inline bool inode_eq_iversion(struct inode *inode, u64 v) { return inode->i_version == v; }
#endif

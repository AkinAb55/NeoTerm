/* uKernel — ext4 extra konstansok (a workflow-agensek gyűjtése a valódi kernel-headerekből). */
#ifndef _UK_EXT4_EXTRA_H
#define _UK_EXT4_EXTRA_H
#ifndef ___GFP_FS
#define ___GFP_FS 0x80u
#endif
#define part_stat_read(part, field) (0UL)
#define part_stat_read_accum(part, field) (0UL)
#include <linux/types.h>

#ifndef ACL_GROUP
#define ACL_GROUP 0x08
#endif
#ifndef ACL_GROUP_OBJ
#define ACL_GROUP_OBJ 0x04
#endif
#ifndef ACL_MASK
#define ACL_MASK 0x10
#endif
#ifndef ACL_OTHER
#define ACL_OTHER 0x20
#endif
#ifndef ACL_USER
#define ACL_USER 0x02
#endif
#ifndef ACL_USER_OBJ
#define ACL_USER_OBJ 0x01
#endif
#ifndef BLK_MAX_SEGMENT_SIZE
#define BLK_MAX_SEGMENT_SIZE 65536
#endif
#ifndef BLK_OPEN_READ
#define BLK_OPEN_READ ((__force blk_mode_t)(1 << 0))
#endif
#ifndef BLK_OPEN_RESTRICT_WRITES
#define BLK_OPEN_RESTRICT_WRITES ((__force blk_mode_t)(1 << 5))
#endif
#ifndef BLK_OPEN_WRITE
#define BLK_OPEN_WRITE ((__force blk_mode_t)(1 << 1))
#endif
#ifndef BLOCK_SIZE
#define BLOCK_SIZE (1<<BLOCK_SIZE_BITS)
#endif
#ifndef BLOCK_SIZE_BITS
#define BLOCK_SIZE_BITS 10
#endif
#ifndef DUMP_PREFIX_ADDRESS
#define DUMP_PREFIX_ADDRESS 1
#endif
#ifndef ENOKEY
#define ENOKEY 126
#endif
#ifndef FGP_NOFS
#define FGP_NOFS 0x00000010
#endif
#ifndef FIEMAP_EXTENT_DELALLOC
#define FIEMAP_EXTENT_DELALLOC 0x00000004
#endif
#ifndef FIEMAP_EXTENT_UNWRITTEN
#define FIEMAP_EXTENT_UNWRITTEN 0x00000800
#endif
#ifndef FIEMAP_FLAG_CACHE
#define FIEMAP_FLAG_CACHE 0x00000004
#endif
#ifndef FMH_IF_VALID
#define FMH_IF_VALID 0
#endif
#ifndef FMH_OF_DEV_T
#define FMH_OF_DEV_T 0x1
#endif
#ifndef FMODE_CAN_ATOMIC_WRITE
#define FMODE_CAN_ATOMIC_WRITE ((__force fmode_t)(1 << 7))
#endif
#ifndef FMR_OF_ATTR_FORK
#define FMR_OF_ATTR_FORK 0x2
#endif
#ifndef FMR_OF_EXTENT_MAP
#define FMR_OF_EXTENT_MAP 0x4
#endif
#ifndef FMR_OF_LAST
#define FMR_OF_LAST 0x20
#endif
#ifndef FMR_OF_PREALLOC
#define FMR_OF_PREALLOC 0x1
#endif
#ifndef FMR_OF_SHARED
#define FMR_OF_SHARED 0x8
#endif
#ifndef FMR_OF_SPECIAL_OWNER
#define FMR_OF_SPECIAL_OWNER 0x10
#endif
#ifndef FMR_OWN_FREE
#define FMR_OWN_FREE FMR_OWNER(0, 1)
#endif
#ifndef FMR_OWN_METADATA
#define FMR_OWN_METADATA FMR_OWNER(0, 3)
#endif
#ifndef FMR_OWN_UNKNOWN
#define FMR_OWN_UNKNOWN FMR_OWNER(0, 2)
#endif
#ifndef FOP_BUFFER_RASYNC
#define FOP_BUFFER_RASYNC ((__force fop_flags_t)(1 << 0))
#endif
#ifndef FOP_DIO_PARALLEL_WRITE
#define FOP_DIO_PARALLEL_WRITE ((__force fop_flags_t)(1 << 3))
#endif
#ifndef FOP_DONTCACHE
#define FOP_DONTCACHE ((__force fop_flags_t)(1 << 7))
#endif
#ifndef FOP_MMAP_SYNC
#define FOP_MMAP_SYNC ((__force fop_flags_t)(1 << 2))
#endif
#ifndef FSCRYPT_SET_CONTEXT_MAX_SIZE
#define FSCRYPT_SET_CONTEXT_MAX_SIZE 40
#endif
#ifndef FS_IOC_GETVERSION
#define FS_IOC_GETVERSION _IOR('v', 1, long)
#endif
#ifndef FS_IOC_GET_ENCRYPTION_NONCE
#define FS_IOC_GET_ENCRYPTION_NONCE _IOR('f', 27, __u8[16])
#endif
#ifndef FS_IOC_GET_ENCRYPTION_POLICY_EX
#define FS_IOC_GET_ENCRYPTION_POLICY_EX _IOWR('f', 22, __u8[9])
#endif
#ifndef FS_IOC_GET_ENCRYPTION_PWSALT
#define FS_IOC_GET_ENCRYPTION_PWSALT _IOW('f', 20, __u8[16])
#endif
#ifndef FS_IOC_SETVERSION
#define FS_IOC_SETVERSION _IOW('v', 2, long)
#endif
#ifndef FS_LBS
#define FS_LBS 128
#endif
#ifndef FS_MGTIME
#define FS_MGTIME 64
#endif
#ifndef FS_PROJINHERIT_FL
#define FS_PROJINHERIT_FL 0x20000000
#endif
#ifndef IOCB_ATOMIC
#define IOCB_ATOMIC 0x00000040
#endif
#ifndef IOMAP_DAX
#define IOMAP_DAX (1 << 8)
#endif
#ifndef IOMAP_DIO_UNWRITTEN
#define IOMAP_DIO_UNWRITTEN (1 << 0)
#endif
#ifndef IOMAP_F_ATOMIC_BIO
#define IOMAP_F_ATOMIC_BIO (1U << 8)
#endif

#ifndef I_MUTEX_NORMAL
#define I_MUTEX_NORMAL 0
#endif
#ifndef I_MUTEX_XATTR
#define I_MUTEX_XATTR 3
#endif
#ifndef MAX_PAGECACHE_ORDER
#define MAX_PAGECACHE_ORDER 8
#endif
#ifndef MBE_REFERENCED_B
#define MBE_REFERENCED_B 0
#endif
#ifndef MBE_REUSABLE_B
#define MBE_REUSABLE_B 1
#endif
#ifndef NR_STAT_GROUPS
#define NR_STAT_GROUPS 4
#endif
#ifndef PF_MEMALLOC
#define PF_MEMALLOC 0x00000800
#endif
#ifndef PF_MEMALLOC_NOFS
#define PF_MEMALLOC_NOFS 0x00040000
#endif
#ifndef QFMT_VFS_OLD
#define QFMT_VFS_OLD 1
#endif
#ifndef QFMT_VFS_V0
#define QFMT_VFS_V0 2
#endif
#ifndef QFMT_VFS_V1
#define QFMT_VFS_V1 4
#endif
#ifndef SB_ENC_NO_COMPAT_FALLBACK_FL
#define SB_ENC_NO_COMPAT_FALLBACK_FL (1 << 1)
#endif
#ifndef SB_ENC_STRICT_MODE_FL
#define SB_ENC_STRICT_MODE_FL (1 << 0)
#endif
#ifndef SB_INLINECRYPT
#define SB_INLINECRYPT 0x00020000
#endif
#ifndef SB_I_ALLOW_HSM
#define SB_I_ALLOW_HSM 0x00004000
#endif
#ifndef SB_I_CGROUPWB
#define SB_I_CGROUPWB 0x00000001
#endif
#ifndef SB_I_VERSION
#define SB_I_VERSION 0x00800000
#endif
#ifndef SB_LAZYTIME
#define SB_LAZYTIME (1 << 25)
#endif
#ifndef SEQ_START_TOKEN
#define SEQ_START_TOKEN ((void *)1)
#endif
#ifndef STATX_ATTR_NODUMP
#define STATX_ATTR_NODUMP 0x00000040
#endif
#ifndef STATX_ATTR_VERITY
#define STATX_ATTR_VERITY 0x00100000
#endif
#ifndef STATX_DIOALIGN
#define STATX_DIOALIGN 0x00002000U
#endif
#ifndef STATX_WRITE_ATOMIC
#define STATX_WRITE_ATOMIC 0x00010000U
#endif
#ifndef STAT_DISCARD
#define STAT_DISCARD 2
#endif
#ifndef STAT_FLUSH
#define STAT_FLUSH 3
#endif
#ifndef STAT_READ
#define STAT_READ 0
#endif
#ifndef STAT_WRITE
#define STAT_WRITE 1
#endif
#ifndef S_DAX
#define S_DAX (1 << 13)
#endif
#ifndef S_NOQUOTA
#define S_NOQUOTA (1 << 5)
#endif
#ifndef VMA_HUGEPAGE_BIT
#define VMA_HUGEPAGE_BIT 29
#endif
#ifndef WHITEOUT_DEV
#define WHITEOUT_DEV 0
#endif
#ifndef WHITEOUT_MODE
#define WHITEOUT_MODE 0
#endif
#ifndef __GFP_FS
#define __GFP_FS ((__force gfp_t)___GFP_FS)
#endif
#ifndef __GFP_MOVABLE
#define __GFP_MOVABLE 0x08u
#endif
#ifndef fs_param_can_be_empty
#define fs_param_can_be_empty 0x0004
#endif
#endif
/* uKernel — ext4/jbd2 futásidő-stubok (auto-generált no-op + valódi impl-ek). */
#include <stddef.h>
long array_index_nospec() { return 0; }
long assert_spin_locked() { return 0; }
long atomic_dec_if_positive() { return 0; }
long bdev_atomic_write_unit_max_bytes() { return 0; }
long bdev_atomic_write_unit_min_bytes() { return 0; }
long bdev_can_atomic_write() { return 0; }
long bdev_dma_alignment() { return 0; }
long bdev_file_open_by_dev() { return 0; }
long bdev_fput() { return 0; }
long bdev_getblk() { return 0; }
long bdev_read_only() { return 0; }
long bdev_rot() { return 0; }
long bdev_write_zeroes_unmap_sectors() { return 0; }
long __bforget() { return 0; }
long bh_offset() { return 0; }
long bio_add_folio() { return 0; }
long bio_max_segs() { return 0; }
long bio_sectors() { return 0; }
long bit_waitqueue() { return 0; }
long blk_crypto_submit_bio() { return 0; }
long blkdev_issue_discard() { return 0; }
long blkdev_issue_zeroout() { return 0; }
long block_commit_write() { return 0; }
long block_is_partially_uptodate() { return 0; }
long block_page_mkwrite() { return 0; }
long buffer_migrate_folio_norefs() { return 0; }
long buffer_write_io_error() { return 0; }
long cache_no_acl() { return 0; }
long clear_buffer_prio() { return 0; }
long clear_buffer_write_io_error() { return 0; }
long cond_resched_lock() { return 0; }
long data_race() { return 0; }
long dax_break_layout_final() { return 0; }
long daxdev_mapping_supported() { return 0; }
long dax_writeback_mapping_range() { return 0; }
long dax_zero_range() { return 0; }
long d_find_any_alias() { return 0; }
long dget_parent() { return 0; }
long dir_relax_shared() { return 0; }
long d_mark_dontcache() { return 0; }
long d_path() { return 0; }
long dquot_alloc_space() { return 0; }
long dquot_alloc_space_nodirty() { return 0; }
long dquot_file_open() { return 0; }
long dquot_free_space() { return 0; }
long dquot_free_space_nodirty() { return 0; }
long dquot_initialize_needed() { return 0; }
long dquot_reclaim_block() { return 0; }
long dquot_resume() { return 0; }
long dquot_suspend() { return 0; }
long dquot_writeback_dquots() { return 0; }
long d_tmpfile() { return 0; }
long errseq_check() { return 0; }
long errseq_check_and_advance() { return 0; }
long fgf_set_order() { return 0; }
/* fiemap_fill_next_extent / fiemap_prep: VALÓDI impl a shim/fs/vfs.c-ben (FIEMAP/filefrag) */
long file_bdev() { return 0; }
long file_check_and_advance_wb_err() { return 0; }
long filemap_dirty_folio() { return 0; }
long filemap_fdatawait_range_keep_errors() { return 0; }
long filemap_get_folios() { return 0; }
long filemap_get_folios_tag() { return 0; }
long filemap_release_folio() { return 0; }
long filemap_write_and_wait() { return 0; }
long filemap_write_and_wait_range() { return 0; }
long file_modified() { return 0; }
long file_path() { return 0; }
long file_update_time() { return 0; }
long file_write_and_wait_range() { return 0; }
long __find_get_block_nonatomic() { return 0; }
long find_inode_by_ino_rcu() { return 0; }
long finish_open_simple() { return 0; }
long FMR_OWNER() { return 0; }
long folio_clear_checked() { return 0; }
long folio_clear_dirty() { return 0; }
long folio_clear_dirty_for_io() { return 0; }
long folio_clear_uptodate() { return 0; }
long folio_end_read() { return 0; }
long folio_end_writeback() { return 0; }
long folio_mapped() { return 0; }
long folio_maybe_dma_pinned() { return 0; }
long folio_mkclean() { return 0; }
long folio_next_index() { return 0; }
long folio_redirty_for_writepage() { return 0; }
long folio_set_checked() { return 0; }
long folio_set_mappedtodisk() { return 0; }
long __folio_start_writeback() { return 0; }
long folio_test_checked() { return 0; }
long folio_test_writeback() { return 0; }
long folio_wait_stable() { return 0; }
long folio_wait_writeback() { return 0; }
long folio_zero_segments() { return 0; }
long folio_zero_tail() { return 0; }
long freezing() { return 0; }
long fscrypt_decrypt_bio() { return 0; }
long fscrypt_drop_inode() { return 0; }
long fscrypt_dummy_policies_equal() { return 0; }
long fscrypt_enqueue_decrypt_work() { return 0; }
long fscrypt_file_open() { return 0; }
long fscrypt_fname_free_buffer() { return 0; }
long fscrypt_free_dummy_policy() { return 0; }
long fscrypt_free_inode() { return 0; }
long fscrypt_ioctl_add_key() { return 0; }
long fscrypt_ioctl_get_key_status() { return 0; }
long fscrypt_ioctl_get_nonce() { return 0; }
long fscrypt_ioctl_get_policy() { return 0; }
long fscrypt_ioctl_get_policy_ex() { return 0; }
long fscrypt_ioctl_remove_key() { return 0; }
long fscrypt_ioctl_remove_key_all_users() { return 0; }
long fscrypt_ioctl_set_policy() { return 0; }
long fscrypt_is_dummy_policy_set() { return 0; }
long fscrypt_is_nokey_name() { return 0; }
long fscrypt_limit_io_blocks() { return 0; }
long fscrypt_prepare_link() { return 0; }
long fscrypt_prepare_new_inode() { return 0; }
long fscrypt_prepare_readdir() { return 0; }
long fscrypt_prepare_rename() { return 0; }
long fscrypt_prepare_setattr() { return 0; }
long fscrypt_prepare_symlink() { return 0; }
long fscrypt_put_encryption_info() { return 0; }
long fscrypt_set_context() { return 0; }
long fscrypt_show_test_dummy_encryption() { return 0; }
long fs_dax_get_by_bdev() { return 0; }
long fserror_report_file_metadata() { return 0; }
long fserror_report_metadata() { return 0; }
long fserror_report_shutdown() { return 0; }
long fs_holder_ops() { return 0; }
long fs_lookup_param() { return 0; }
long fs_param_is_blockdev() { return 0; }
long fs_put_dax() { return 0; }
long fsverity_ioctl_enable() { return 0; }
long fsverity_ioctl_measure() { return 0; }
long fsverity_ioctl_read_metadata() { return 0; }
long generic_atomic_write_valid() { return 0; }
long generic_check_addressable() { return 0; }
long generic_error_remove_folio() { return 0; }
long generic_file_llseek_size() { return 0; }
long generic_fill_statx_atomic_writes() { return 0; }
long generic_perform_write() { return 0; }
long generic_set_sb_d_ops() { return 0; }
long get_current_ioprio() { return 0; }
long get_cycles() { return 0; }
long icount_read() { return 0; }
long in_group_p() { return 0; }
long inode_fsuid_set() { return 0; }
long inode_generic_drop() { return 0; }
long inode_get_atime_sec() { return 0; }
long inode_get_ctime_sec() { return 0; }
long inode_get_mtime_sec() { return 0; }
long inode_io_list_del() { return 0; }
long inode_is_dirtytime_only() { return 0; }
long inode_is_open_for_write() { return 0; }
long inode_lock_nested() { return 0; }
/* inode_set_cached_link: VALÓDI impl a vfs.c-ben (i_link/i_linklen) — a fast-symlink readlinkhez */
long inode_set_flags() { return 0; }
long inode_state_clear() { return 0; }
long inode_trylock_shared() { return 0; }
long invalidate_bdev() { return 0; }
long invalidate_mapping_pages() { return 0; }
long iocb_bio_iopoll() { return 0; }
long iomap_dio_rw() { return 0; }
/* iomap_fiemap: VALÓDI impl a vfs.c-ben (bmap-alapú extent-enumeráció) */
long iomap_seek_data() { return 0; }
long iomap_seek_hole() { return 0; }
long iomap_swapfile_activate() { return 0; }
long iov_iter_truncate() { return 0; }
long IS_CASEFOLDED() { return 0; }
long IS_DAX() { return 0; }
long IS_NOQUOTA() { return 0; }
long IS_NOSEC() { return 0; }
long is_quota_modification() { return 0; }
long IS_SWAPFILE() { return 0; }
long kfree_link() { return 0; }
long kmemdup_nul() { return 0; }
long __kprojid_val() { return 0; }
long ktime_add_ns() { return 0; }
long ktime_get() { return 0; }
long ktime_get_coarse_real_ts64() { return 0; }
long ktime_get_ns() { return 0; }
/* VALÓDI Unix-idő — az ext4 ezt írja az i_dtime-ba törléskor; a 0-stub miatt a fsck
 * "Deleted inode has zero dtime"-ot jelzett. (libc time().) */
extern long time(long *);
long ktime_get_real_seconds() { return time((long *)0); }
long ktime_sub() { return 0; }
long list_add_tail_rcu() { return 0; }
long list_del_rcu() { return 0; }
long list_empty_careful() { return 0; }
long list_replace_init() { return 0; }
long list_sort() { return 0; }
long lockdep_assert_held_read() { return 0; }
long lockdep_assert_held_write() { return 0; }
long lockdep_register_key() { return 0; }
long lockdep_unregister_key() { return 0; }
long lock_two_nondirectories() { return 0; }
long MAJOR() { return 0; }
long mapping_gfp_constraint() { return 0; }
long mapping_gfp_mask() { return 0; }
long mapping_max_folio_order() { return 0; }
long mapping_set_error() { return 0; }
long mapping_set_folio_order_range() { return 0; }
long mapping_tagged() { return 0; }
long mb_cache_destroy() { return 0; }
long mb_cache_entry_create() { return 0; }
long mb_cache_entry_delete_or_get() { return 0; }
long mb_cache_entry_find_first() { return 0; }
long mb_cache_entry_find_next() { return 0; }
long mb_cache_entry_get() { return 0; }
long mb_cache_entry_put() { return 0; }
long mb_cache_entry_touch() { return 0; }
long mb_cache_entry_wait_unused() { return 0; }
long memalloc_nofs_restore() { return 0; }
long memalloc_nofs_save() { return 0; }
long memalloc_retry_wait() { return 0; }
long memtostr_pad() { return 0; }
long might_sleep() { return 0; }
long MINOR() { return 0; }
long mmb_has_buffers() { return 0; }
long nd_terminate_link() { return 0; }
long noop_dirty_folio() { return 0; }
long no_printk() { return 0; }
long nsecs_to_jiffies() { return 0; }
long old_decode_dev() { return 0; }
long old_encode_dev() { return 0; }
long old_valid_dev() { return 0; }
long pagecache_isize_extended() { return 0; }
long page_cache_sync_readahead() { return 0; }
long page_folio() { return 0; }
long path_put() { return 0; }
long posix_acl_chmod() { return 0; }
/* posix_acl_alloc / posix_acl_release / posix_acl_update_mode: VALÓDI impl a shim/fs/posix_acl.c-ben */
long prefetchw() { return 0; }
long prepare_to_wait_exclusive() { return 0; }
long pr_info_ratelimited() { return 0; }
long print_hex_dump() { return 0; }
long printk_once() { return 0; }
long pr_notice_ratelimited() { return 0; }
long proc_create_seq_data() { return 0; }
long proc_create_single_data() { return 0; }
long projid_eq() { return 0; }
long pr_warn_once() { return 0; }
long ra_has_index() { return 0; }
long release_dentry_name_snapshot() { return 0; }
long remove_proc_subtree() { return 0; }
long round_jiffies_up() { return 0; }
long sb_any_quota_suspended() { return 0; }
long sb_bdev_nr_blocks() { return 0; }
long sb_end_intwrite() { return 0; }
long sb_end_pagefault() { return 0; }
long sb_end_write() { return 0; }
long sb_find_get_block_nonatomic() { return 0; }
/* a shim metaadat-bufferei block-device-bufferek (dummy bdev-folio) -> a jbd2-revoke
 * az inode-adat-agat atugorja; csak igy nem deref-eli a nem-letezo data-mappinget. */
long sb_is_blkdev_sb() { return 1; }
long sb_issue_zeroout() { return 0; }
long sb_no_casefold_compat_fallback() { return 0; }
long sb_start_intwrite() { return 0; }
long sb_start_intwrite_trylock() { return 0; }
long sb_start_pagefault() { return 0; }
long sb_start_write_trylock() { return 0; }
long schedule_hrtimeout() { return 0; }
long secs_to_jiffies() { return 0; }
long set_blocksize() { return 0; }
long set_buffer_prio() { return 0; }
/* set_cached_acl: VALÓDI impl a shim/fs/posix_acl.c-ben (inode->i_acl/i_default_acl cache) */
long set_delayed_call() { return 0; }
long set_freezable() { return 0; }
/* simple_get_link: VALÓDI impl a vfs.c-ben (return inode->i_link) — a readlinkhez kell */
long sort() { return 0; }
int spin_is_locked(void *l) { (void)l; return 0; }   /* sosem zárolt (egyszálú) */
long spin_needbreak() { return 0; }
int spin_trylock(void *l) { (void)l; return 1; }   /* egyszálú shim: a zár mindig megszerezhető */
long strreplace() { return 0; }
long strscpy_pad() { return 0; }
long strtomem_pad() { return 0; }
long str_yes_no() { return 0; }
long super_set_sysfs_name_bdev() { return 0; }
long sysfs_notify() { return 0; }
long tag_pages_for_writeback() { return 0; }
long take_dentry_name_snapshot() { return 0; }
long task_pid_nr() { return 0; }
long test_clear_buffer_dirty() { return 0; }
long thp_get_unmapped_area() { return 0; }
long time_before32() { return 0; }
long time_is_before_jiffies() { return 0; }
long truncate_inode_pages_range() { return 0; }
long truncate_pagecache_range() { return 0; }
long try_to_free_buffers() { return 0; }
long try_to_freeze() { return 0; }
long try_to_writeback_inodes_sb() { return 0; }
long unlock_two_nondirectories() { return 0; }
long unmap_mapping_pages() { return 0; }
long uuid_is_null() { return 0; }
long vfs_setpos() { return 0; }
long vma_desc_set_flags() { return 0; }
long wait_on_bit_io() { return 0; }
long WARN_ONCE() { return 0; }
long WARN_RATELIMIT() { return 0; }
long wbc_account_cgroup_owner() { return 0; }
long wbc_init_bio() { return 0; }
long wbc_to_tag() { return 0; }
long writeback_iter() { return 0; }
long write_trylock() { return 0; }
long xa_destroy() { return 0; }
long xa_erase() { return 0; }
long xa_insert() { return 0; }
long xa_load() { return 0; }
long ZERO_OR_NULL_PTR() { return 0; }

/* ===== VALÓDI implementációk ===== */
#include <stdint.h>
#include <string.h>
long long atomic64_read(const void *v){ return *(const long long*)v; }
void atomic64_set(void *v, long long i){ *(long long*)v=i; }
void atomic64_add(long long i, void *v){ *(long long*)v+=i; }
void atomic64_sub(long long i, void *v){ *(long long*)v-=i; }
void atomic64_inc(void *v){ (*(long long*)v)++; }
static uint64_t uk_bswap64(uint64_t x){ return ((x&0xffULL)<<56)|((x&0xff00ULL)<<40)|((x&0xff0000ULL)<<24)|((x&0xff000000ULL)<<8)|((x>>8)&0xff000000ULL)|((x>>24)&0xff0000ULL)|((x>>40)&0xff00ULL)|((x>>56)&0xffULL); }
uint64_t be64_to_cpu(uint64_t x){ return uk_bswap64(x); }
uint64_t cpu_to_be64(uint64_t x){ return uk_bswap64(x); }
uint64_t div64_u64(uint64_t a, uint64_t b){ return b? a/b:0; }
uint64_t div_u64(uint64_t a, uint32_t b){ return b? a/b:0; }
uint64_t DIV_ROUND_UP_ULL(uint64_t a, uint32_t b){ return b? (a+b-1)/b:0; }
uint32_t rol32(uint32_t w, unsigned s){ return (w<<(s&31))|(w>>((-s)&31)); }
uint32_t get_random_u32_below(uint32_t c){ (void)c; return 0; }
unsigned long memweight(const void *ptr, size_t n){ unsigned long c=0; const unsigned char*p=ptr; for(size_t i=0;i<n;i++){unsigned char b=p[i];while(b){c+=b&1;b>>=1;}} return c; }
int test_bit_le(int nr, const void *a){ return (((const unsigned char*)a)[nr>>3]>>(nr&7))&1; }
int __test_and_set_bit_le(int nr, void *a){ unsigned char*p=&((unsigned char*)a)[nr>>3]; int o=(*p>>(nr&7))&1; *p|=(1<<(nr&7)); return o; }
int __test_and_clear_bit_le(int nr, void *a){ unsigned char*p=&((unsigned char*)a)[nr>>3]; int o=(*p>>(nr&7))&1; *p&=~(1<<(nr&7)); return o; }
void __set_bit_le(int nr, void *a){ ((unsigned char*)a)[nr>>3]|=(1<<(nr&7)); }
void __clear_bit_le(int nr, void *a){ ((unsigned char*)a)[nr>>3]&=~(1<<(nr&7)); }
void le16_add_cpu(uint16_t *v, uint16_t a){ *v=(uint16_t)(*v+a); }
void le32_add_cpu(uint32_t *v, uint32_t a){ *v+=a; }
void le64_add_cpu(uint64_t *v, uint64_t a){ *v+=a; }
/* crc32c: a KERNEL-alak (nyers reflektált CRC32C-loop, BELSŐ inverzió NÉLKÜL) — az ext4 a
 * crc32c(~0, data, len)-t hívja és a seedet/inverziót maga kezeli. A korábbi invertáló alak
 * (crc=~crc ... return ~crc) a metadata_csum-superblock-verifyt elbuktatta. */
uint32_t crc32c(uint32_t crc, const void *data, size_t len){ const unsigned char*p=data; for(size_t i=0;i<len;i++){crc^=p[i]; for(int k=0;k<8;k++) crc=(crc>>1)^(0x82F63B78 & -(crc&1));} return crc; }
uint32_t crc32_be(uint32_t crc, const void *data, size_t len){ const unsigned char*p=data; for(size_t i=0;i<len;i++){crc^=(uint32_t)p[i]<<24; for(int k=0;k<8;k++) crc=(crc&0x80000000)?(crc<<1)^0x04C11DB7:(crc<<1);} return crc; }
struct uk_qstr { unsigned int hash_len; const unsigned char *name; };
const struct uk_qstr dotdot_name = { 2, (const unsigned char*)".." };
char nop_posix_acl_access[64];
char nop_posix_acl_default[64];

/* ===== mempool (valódi — a slab-cache köré) ===== */
#include <stdlib.h>
struct uk_mempool { void *(*alloc_fn)(unsigned, void*); void (*free_fn)(void*, void*); void *pool_data; };
void *mempool_alloc_slab(unsigned gfp, void *cache);   /* a slab.c-ből (kmem_cache_alloc) */
void mempool_free_slab(void *e, void *cache);
void *mempool_create(int min_nr, void *alloc_fn, void *free_fn, void *pool_data){
	(void)min_nr; struct uk_mempool *p = malloc(sizeof(*p));
	if (p){ p->alloc_fn=alloc_fn; p->free_fn=free_fn; p->pool_data=pool_data; } return p;
}
void mempool_destroy(void *pool){ free(pool); }
void *mempool_alloc(void *pool, unsigned gfp){ struct uk_mempool *p=pool; return p&&p->alloc_fn? p->alloc_fn(gfp, p->pool_data):0; }
void mempool_free(void *e, void *pool){ struct uk_mempool *p=pool; if(p&&p->free_fn) p->free_fn(e, p->pool_data); }
/* mempool_alloc_slab/free_slab a kmem_cache-en át (a slab.c kmem_cache_alloc/free-jét hívjuk) */
void *kmem_cache_alloc(void *c, unsigned f);
void kmem_cache_free(void *c, void *o);
void *mempool_alloc_slab(unsigned gfp, void *cache){ return kmem_cache_alloc(cache, gfp); }
void mempool_free_slab(void *e, void *cache){ kmem_cache_free(cache, e); }

/* mb_cache (metaadat-blokk dedup cache): nem-NULL dummy — a find-ek NULL-t (miss) adnak,
 * ami azt jelenti "nincs dedup", funkcionálisan helyes (csak nincs xattr-megosztás). */
void *mb_cache_create(int bucket_bits){ (void)bucket_bits; static int dummy; return &dummy; }

/* -O0 fordításnál előbukkanó titkosítás/dax szimbólumok (nem használt utak) */
long dax_break_layout_inode() { return 0; }
long fscrypt_decrypt_pagecache_blocks() { return 0; }
long fscrypt_dio_supported() { return 0; }
long fscrypt_encrypt_symlink() { return 0; }
long fscrypt_fname_alloc_buffer() { return 0; }
long fscrypt_fname_disk_to_usr() { return 0; }
long fscrypt_has_permitted_context() { return 0; }
long fscrypt_zeroout_range() { return 0; }

/* posix_acl_create: a kimeneti default_acl/acl pointereket NULL-ra kell állítani (nincs ACL),
 * különben az ext4_init_acl szemét pointerrel dolgozik → -EACCES. */
int posix_acl_create(void *dir, unsigned short *mode, void **default_acl, void **acl)
{ (void)dir; (void)mode; if (default_acl) *default_acl = NULL; if (acl) *acl = NULL; return 0; }

/* generic_ci_validate_strict_name: nem-casefold (CONFIG_UNICODE nélkül) FS-en a név mindig
 * érvényes → 1 (true). A no-op 0 (false) -EINVAL-t okozott a dir-entry beszúrásnál. */
int generic_ci_validate_strict_name(void *dir, void *name) { (void)dir; (void)name; return 1; }

/* uKernel — ntfs3 futásidő-stubok (auto-generált). A legtöbb return 0 (NULL),
 * elég ahhoz hogy a .so betöltsön és az üres-mount read-útja elinduljon. */
#include <stddef.h>
typedef unsigned long ulong;

long blkdev_issue_discard() { return 0; }
long blkdev_issue_zeroout() { return 0; }
long clean_bdev_aliases() { return 0; }
long copy_folio_from_iter_atomic() { return 0; }
long dentry_path_raw() { return 0; }
long discard_new_inode() { return 0; }
long fault_in_iov_iter_readable() { return 0; }
long filemap_migrate_folio() { return 0; }
long filemap_write_and_wait_range() { return 0; }
long file_modified() { return 0; }
long file_ra_state_init() { return 0; }
long file_remove_privs() { return 0; }
long file_update_time() { return 0; }
long file_write_and_wait_range() { return 0; }
long flush_dcache_folio() { return 0; }
long flush_dcache_page() { return 0; }
long folio_clear_dirty() { return 0; }
long folio_fill_tail() { return 0; }
/* folio_pos, folio_unlock, folio_zero_range, folio_zero_segment -> vfs.c (valódi impl.) */
long fs_umode_to_dtype() { return 0; }
long generic_file_llseek_size() { return 0; }
long generic_file_open() { return 0; }
long global_zone_page_state() { return 0; }
long i_gid_read() { return 0; }
long i_gid_write() { return 0; }
/* inode_get_bytes: VALÓDI impl a vfs.c-ben */
long inode_nohighmem() { return 0; }
/* inode_set_bytes: VALÓDI impl a vfs.c-ben (i_blocks az ntfs3 alloc_size-ból) */
long inode_trylock_shared() { return 0; }
long invalidate_bdev() { return 0; }
long invalidate_mapping_pages() { return 0; }
long iomap_add_to_ioend() { return 0; }
long iomap_bio_read_folio_range() { return 0; }
long iomap_dio_rw() { return 0; }
long iomap_dirty_folio() { return 0; }
/* iomap_fiemap: VALÓDI impl a vfs.c-ben (bmap-alapú extent-enumeráció) */
long iomap_invalidate_folio() { return 0; }
long iomap_ioend_writeback_submit() { return 0; }
long iomap_readahead() { return 0; }
/* iomap_read_folio -> vfs.c (valódi impl.: iomap_begin(READ) -> INLINE/MAPPED/HOLE) */
long iomap_release_folio() { return 0; }
long iomap_writepages() { return 0; }
long iomap_zero_range() { return 0; }
long iov_iter_count() { return 0; }
long i_uid_read() { return 0; }
long i_uid_write() { return 0; }
long kfree_link() { return 0; }
long le16_add_cpu() { return 0; }
long le32_add_cpu() { return 0; }
long le64_add_cpu() { return 0; }
long lock_page() { return 0; }
long mapping_gfp_mask() { return 0; }
long mapping_set_error() { return 0; }
long mapping_tagged() { return 0; }
long mark_inode_dirty_sync() { return 0; }
long mutex_lock_nested() { return 0; }
long offset_in_page() { return 0; }
long page_cache_sync_readahead() { return 0; }
long page_folio() { return 0; }
long posix_acl_chmod() { return 0; }
long ra_has_index() { return 0; }
long ratelimit_state_init() { return 0; }
long seq_user_ns() { return 0; }
long set_delayed_call() { return 0; }
long unlock_page() { return 0; }
long unsafe_memcpy() { return 0; }
long vfs_setpos() { return 0; }
long vma_desc_size() { return 0; }
long vma_desc_test() { return 0; }
long vmap() { return 0; }
long vunmap() { return 0; }
long writeback_iter() { return 0; }

/* hiányzó __-szimbólumok (a tiszta linkhez, ignore-all nélkül) */
void __clear_bit_le(long nr, void *addr) { ((unsigned char*)addr)[nr>>3] &= ~(1<<(nr&7)); }
void __set_bit_le(long nr, void *addr)   { ((unsigned char*)addr)[nr>>3] |= (1<<(nr&7)); }
void __swab16s(unsigned short *p) { if(p) *p = (*p<<8)|(*p>>8); }
void __swab32s(unsigned int *p) { (void)p; }
void __folio_set_locked(void *f) { (void)f; }

/* big-endian in-place konverterek (little-endian hoston = byteswap) */
void __cpu_to_be16s(unsigned short *p) { if(p) *p = (unsigned short)((*p<<8)|(*p>>8)); }
void __be16_to_cpus(unsigned short *p) { if(p) *p = (unsigned short)((*p<<8)|(*p>>8)); }
void __cpu_to_be32s(unsigned int *p) { if(p){ unsigned int v=*p; *p=((v&0xff)<<24)|((v&0xff00)<<8)|((v>>8)&0xff00)|((v>>24)&0xff); } }
void __be32_to_cpus(unsigned int *p) { if(p){ unsigned int v=*p; *p=((v&0xff)<<24)|((v&0xff00)<<8)|((v>>8)&0xff00)|((v>>24)&0xff); } }
void __cpu_to_le16s(unsigned short *p) { (void)p; }
void __cpu_to_le32s(unsigned int *p) { (void)p; }
void __cpu_to_le64s(unsigned long long *p) { (void)p; }
void __le16_to_cpus(unsigned short *p) { (void)p; }
void __le32_to_cpus(unsigned int *p) { (void)p; }

/* hex_asc: a lib/hexdump extern tömbjei (-O0 ntfs3-fordításnál nem inline-olódnak) */
const char hex_asc[] = "0123456789abcdef";
const char hex_asc_upper[] = "0123456789ABCDEF";
/* bitkereső helperek (a kernel-bitops makrók -O0-nál külső hívásként jelennek meg) */
unsigned long __ffs(unsigned long x) { return (unsigned long)__builtin_ctzl(x); }
unsigned long __fls(unsigned long x) { return (unsigned long)(63 - __builtin_clzl(x)); }
unsigned long ffz(unsigned long x)   { return (unsigned long)__builtin_ctzl(~x); }
/* folio_file_page: a shimben folio==page, így a folio-t adjuk vissza (index irreleváns) */
void *folio_file_page(void *folio, unsigned long index) { (void)index; return folio; }

/* uKernel hamis <linux/pagemap.h> — page cache felület. A blokk-alapú segédek a
 * buffer_head.h-ban vannak; itt a filemap-* és a könyvtár-lapozás. */
#ifndef _UK_LINUX_PAGEMAP_H
#define _UK_LINUX_PAGEMAP_H
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include <linux/mpage.h>

struct page;
struct folio;
struct address_space;
struct readahead_control;

#define AS_EIO		0
/* FGP_* — a VALÓDI kernel-értékek (eddig FGP_LOCK/ACCESSED fel volt cserélve, és
 * FGP_WRITEBEGIN rossz volt → a write_begin folio NEM zárolódott → inline-írás BUG_ON). */
#define FGP_ACCESSED 0x01
#define FGP_LOCK     0x02
#define FGP_CREAT    0x04
#define FGP_WRITE    0x08
#define FGP_NOFS     0x10
#define FGP_NOWAIT   0x20
#define FGP_FOR_MMAP 0x40
#define FGP_STABLE   0x80
#define PAGE_CACHE_SIZE	PAGE_SIZE

int generic_write_end(const struct kiocb *, struct address_space *, loff_t, unsigned, unsigned, struct folio *, void *);

/* ext4 folio-segédek (pointer-visszatérés — prototípus KELL, különben int-csonkolás 64-biten) */
struct folio *filemap_get_folio(struct address_space *mapping, pgoff_t index);
struct folio *filemap_lock_folio(struct address_space *mapping, pgoff_t index);
struct folio *write_begin_get_folio(const struct kiocb *iocb, struct address_space *mapping, pgoff_t index, size_t len);
struct folio *__filemap_get_folio(struct address_space *mapping, pgoff_t index, int fgp_flags, gfp_t gfp);

int filemap_fdatawrite(struct address_space *);
int filemap_fdatawait_range(struct address_space *, loff_t, loff_t);
ssize_t filemap_splice_read(struct file *in, loff_t *ppos, struct pipe_inode_info *pipe,
			    size_t len, unsigned int flags);

void mapping_set_error(struct address_space *mapping, int error);
static inline struct inode *mapping_host(struct address_space *m) { return m->host; }
static inline unsigned long dir_pages(struct inode *inode)
{ return (i_size_read(inode) + PAGE_SIZE - 1) >> PAGE_SHIFT; }

#ifndef _UK_VM_FAULT
#define _UK_VM_FAULT
struct vm_area_struct { struct file *vm_file; unsigned long vm_flags; struct address_space *vm_mapping; };
struct vm_fault { struct vm_area_struct *vma; unsigned long address; struct folio *folio; void *page; unsigned int flags; };
struct vm_operations_struct {
	int (*fault)(struct vm_fault *vmf);
	int (*page_mkwrite)(struct vm_fault *vmf);
	void (*map_pages)(struct vm_fault *vmf, unsigned long start, unsigned long end);
};
int filemap_fault(struct vm_fault *vmf);
void filemap_map_pages(struct vm_fault *vmf, unsigned long start, unsigned long end);

#ifndef PAGECACHE_TAG_DIRTY
#define PAGECACHE_TAG_DIRTY 0
#define PAGECACHE_TAG_WRITEBACK 1
#endif
#ifndef FGP_WRITEBEGIN
#define FGP_WRITEBEGIN (FGP_LOCK | FGP_WRITE | FGP_CREAT | FGP_STABLE)
#endif









int filemap_migrate_folio(struct address_space *m, struct folio *dst, struct folio *src, int mode);
bool filemap_dirty_folio(struct address_space *mapping, struct folio *folio);
int filemap_write_and_wait_range(struct address_space *mapping, loff_t start, loff_t end);
#endif
int filemap_flush(struct address_space *mapping);
extern const struct address_space_operations empty_aops;
#endif

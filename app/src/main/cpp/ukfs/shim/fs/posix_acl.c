/* uKernel — VALÓDI POSIX ACL mag (a kernel fs/posix_acl.c megfelelő részeinek hű másolata).
 * Az ext4/acl.c (ext4_get_acl/ext4_set_acl + saját lemez-formátum) MÁR valódi és fordul; ez a
 * fájl a posix_acl-mag (alloc/release/clone/cache + xattr<->acl konverzió + mode-egyenértékűség)
 * eddig no-op stubjait cseréli VALÓDIra, hogy a setfacl/getfacl ténylegesen működjön. */
#include <linux/fs.h>
#include <linux/posix_acl.h>
#include <linux/posix_acl_xattr.h>
#include <linux/uidgid.h>
#include <linux/slab.h>
#include <linux/errno.h>   /* ERR_PTR */
#include <stdlib.h>
#include <string.h>

#ifndef S_IRWXUGO
#define S_IRWXUGO (S_IRWXU|S_IRWXG|S_IRWXO)
#endif

/* ===== alloc / init / clone / release (refcount-alapú) ===== */
static void posix_acl_init(struct posix_acl *acl, int count)
{ atomic_set(&acl->a_refcount, 1); acl->a_count = count; }

struct posix_acl *posix_acl_alloc(unsigned int count, gfp_t flags)
{
	(void)flags;
	struct posix_acl *acl = malloc(sizeof(*acl) + count * sizeof(struct posix_acl_entry));
	if (acl) { memset(acl, 0, sizeof(*acl) + count * sizeof(struct posix_acl_entry)); posix_acl_init(acl, count); }
	return acl;
}

struct posix_acl *posix_acl_clone(const struct posix_acl *acl, gfp_t flags)
{
	(void)flags;
	if (!acl) return NULL;
	size_t sz = sizeof(*acl) + acl->a_count * sizeof(struct posix_acl_entry);
	struct posix_acl *clone = malloc(sz);
	if (clone) { memcpy(clone, acl, sz); atomic_set(&clone->a_refcount, 1); }
	return clone;
}

void posix_acl_release(struct posix_acl *acl)
{
	if (!acl || acl == ACL_NOT_CACHED || acl == ACL_DONT_CACHE) return;
	if (atomic_dec_and_test(&acl->a_refcount)) free(acl);
}

/* ===== az inode ACL-cache-e (i_acl / i_default_acl) ===== */
void set_cached_acl(struct inode *inode, int type, struct posix_acl *acl)
{
	struct posix_acl *old = NULL;
	if (acl) atomic_inc(&acl->a_refcount);
	if (type == ACL_TYPE_ACCESS) { old = inode->i_acl; inode->i_acl = acl; }
	else if (type == ACL_TYPE_DEFAULT) { old = inode->i_default_acl; inode->i_default_acl = acl; }
	if (old != ACL_NOT_CACHED) posix_acl_release(old);
}

/* ===== mode-egyenértékűség: ha az ACL kifejezhető tiszta jogbitekkel (nincs named user/group/mask),
 * akkor a mode-ot adja vissza és a hívó eldobhatja a külön ACL-t (a kernel posix_acl_equiv_mode-ja) */
int posix_acl_equiv_mode(const struct posix_acl *acl, umode_t *mode_p)
{
	umode_t mode = 0; int not_equiv = 0;
	if (!acl) return 0;
	for (unsigned i = 0; i < acl->a_count; i++) {
		const struct posix_acl_entry *pa = &acl->a_entries[i];
		switch (pa->e_tag) {
		case ACL_USER_OBJ:  mode |= (pa->e_perm & S_IRWXO) << 6; break;
		case ACL_GROUP_OBJ: mode |= (pa->e_perm & S_IRWXO) << 3; break;
		case ACL_OTHER:     mode |= pa->e_perm & S_IRWXO; break;
		case ACL_MASK:      mode = (mode & ~S_IRWXG) | ((pa->e_perm & S_IRWXO) << 3); not_equiv = 1; break;
		case ACL_USER:
		case ACL_GROUP:     not_equiv = 1; break;
		default: return -22;   /* -EINVAL */
		}
	}
	if (mode_p) *mode_p = (*mode_p & ~S_IRWXUGO) | mode;
	return not_equiv;
}

/* a set_acl-útvonalon az ACCESS-ACL beállításakor a mode-bitek frissítése (a kernel megfelelője,
 * az idmap SGID-csoport-ellenőrzés nélkül — a shim egyfelhasználós/root). */
int posix_acl_update_mode(struct mnt_idmap *idmap, struct inode *inode, umode_t *mode_p, struct posix_acl **acl)
{
	(void)idmap;
	umode_t mode = inode->i_mode;
	int error = posix_acl_equiv_mode(*acl, &mode);
	if (error < 0) return error;
	if (error == 0) *acl = NULL;   /* tiszta jogbitekkel kifejezhető → nincs szükség külön ACL-re */
	*mode_p = mode;
	return 0;
}

/* ===== xattr-bináris <-> struct posix_acl konverzió (a kernel posix_acl_from/to_xattr-ja) ===== */
struct posix_acl *posix_acl_from_xattr(struct user_namespace *userns, const void *value, size_t size)
{
	const struct posix_acl_xattr_header *header = value;
	if (!header) return ERR_PTR(-22);
	if (size < sizeof(*header)) return ERR_PTR(-22);
	if (header->a_version != cpu_to_le32(POSIX_ACL_XATTR_VERSION)) return ERR_PTR(-95);
	int count = posix_acl_xattr_count(size);
	if (count < 0) return ERR_PTR(-22);
	if (count == 0) return NULL;

	const struct posix_acl_xattr_entry *entry = (const void *)(header + 1), *end;
	struct posix_acl *acl = posix_acl_alloc(count, 0);
	if (!acl) return ERR_PTR(-12);   /* -ENOMEM */
	struct posix_acl_entry *acl_e = acl->a_entries;
	for (end = entry + count; entry != end; acl_e++, entry++) {
		acl_e->e_tag  = le16_to_cpu(entry->e_tag);
		acl_e->e_perm = le16_to_cpu(entry->e_perm);
		switch (acl_e->e_tag) {
		case ACL_USER_OBJ: case ACL_GROUP_OBJ: case ACL_MASK: case ACL_OTHER: break;
		case ACL_USER:  acl_e->e_uid = make_kuid(userns, le32_to_cpu(entry->e_id)); if (!uid_valid(acl_e->e_uid)) goto fail; break;
		case ACL_GROUP: acl_e->e_gid = make_kgid(userns, le32_to_cpu(entry->e_id)); if (!gid_valid(acl_e->e_gid)) goto fail; break;
		default: goto fail;
		}
	}
	return acl;
fail:
	posix_acl_release(acl);
	return ERR_PTR(-22);
}

/* a shim-szignatúra (régi alak): a buffer-be ír, a SZÜKSÉGES méretet adja vissza (size==0 -> csak méret),
 * -ERANGE ha a buffer kicsi. (Az ext4 a SAJÁT lemez-formátumát használja, ezt csak az ukfs_getxattr.) */
int posix_acl_to_xattr(struct user_namespace *user_ns, const struct posix_acl *acl, void *buffer, size_t size)
{
	size_t real = posix_acl_xattr_size(acl->a_count);
	if (size == 0) return (int)real;          /* csak a szükséges méret lekérdezése */
	if (size < real) return -34;              /* -ERANGE */
	struct posix_acl_xattr_header *ext = buffer;
	struct posix_acl_xattr_entry *e = (void *)(ext + 1);
	ext->a_version = cpu_to_le32(POSIX_ACL_XATTR_VERSION);
	for (unsigned n = 0; n < acl->a_count; n++, e++) {
		const struct posix_acl_entry *acl_e = &acl->a_entries[n];
		e->e_tag  = cpu_to_le16(acl_e->e_tag);
		e->e_perm = cpu_to_le16(acl_e->e_perm);
		switch (acl_e->e_tag) {
		case ACL_USER:  e->e_id = cpu_to_le32(from_kuid(user_ns, acl_e->e_uid)); break;
		case ACL_GROUP: e->e_id = cpu_to_le32(from_kgid(user_ns, acl_e->e_gid)); break;
		default:        e->e_id = cpu_to_le32(ACL_UNDEFINED_ID); break;
		}
	}
	return (int)real;
}

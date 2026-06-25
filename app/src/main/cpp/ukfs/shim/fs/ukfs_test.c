/* uKernel — FS teszt-harness: mountol egy fájlrendszert és listázza a gyökeret.
 *   ukfs_test <fstype> <devpath/image>
 * A valódi FS-driver (vfat.so beforditva) module_init-je regisztrál, majd uk_mount
 * meghívja a fill_super-t (sb_bread a backenden), és a gyökér iterate_shared-jét
 * futtatjuk egy printelő actorral. */
#define _GNU_SOURCE
#include <linux/fs.h>
#include <stdio.h>
#include <string.h>

extern int ukernel_run_module_inits(void);
extern struct dentry *uk_mount(const char *fstype, const char *devpath);
extern long ukfs_write_file(const char *name, const char *data, size_t len);
extern ssize_t uk_read(struct dentry *dentry, char *buf, size_t len, loff_t pos);
extern int uk_create_write(struct dentry *root, const char *name, const char *data, size_t len);

static char g_names[64][256]; static int g_nnames;

static int print_actor(struct dir_context *ctx, const char *name, int namelen,
		       loff_t off, u64 ino, unsigned type)
{
	(void)ctx; (void)off;
	const char *t = type == DT_DIR ? "DIR " : type == DT_REG ? "FILE" : "    ";
	printf("   [%s] %.*s  (ino=%llu)\n", t, namelen, name, (unsigned long long)ino);
	if (type == DT_REG && g_nnames < 64 && namelen < 256) {
		memcpy(g_names[g_nnames], name, namelen); g_names[g_nnames][namelen] = 0; g_nnames++;
	}
	return 0;
}

int main(int argc, char **argv)
{
	if (argc < 3) { fprintf(stderr, "haszn: %s <fstype> <devpath>\n", argv[0]); return 2; }
	setvbuf(stdout, NULL, _IONBF, 0);

	printf("=== uKernel FS: module_init futtatása ===\n");
	ukernel_run_module_inits();

	printf("=== mount: %s <- %s ===\n", argv[1], argv[2]);
	struct dentry *root = uk_mount(argv[1], argv[2]);
	if (!root || !root->d_inode) { fprintf(stderr, "MOUNT HIBA\n"); return 1; }

	struct inode *dir = root->d_inode;
	printf("=== gyökér-inode: mode=%o size=%lld i_fop=%p ===\n",
	       dir->i_mode, (long long)dir->i_size, (void *)dir->i_fop);
	if (!dir->i_fop || !dir->i_fop->iterate_shared) { fprintf(stderr, "nincs iterate_shared\n"); return 1; }

	printf("=== gyökér-könyvtár tartalma (a VALÓDI FAT-driverrel olvasva) ===\n");
	struct file f;
	memset(&f, 0, sizeof(f));
	f.f_inode = dir; f.f_path.dentry = root; f.f_op = dir->i_fop; f.f_mapping = dir->i_mapping;
	/* az ext4 a dir_private_info-t az open-ben allokálja (file->private_data) — a kernel
	 * is meghívja iterate előtt; vfat/ntfs3-nál nincs open, ott no-op. */
	if (f.f_op->open) f.f_op->open(dir, &f);
	struct dir_context ctx = { .actor = print_actor, .pos = 0 };
	int r = dir->i_fop->iterate_shared(&f, &ctx);
	printf("=== iterate_shared = %d ===\n", r);

	if (!dir->i_op || !dir->i_op->lookup) { printf("(nincs lookup — cat kihagyva)\n"); return 0; }
	printf("\n=== cat: a fájlok tartalma (a VALÓDI vfat-driverrel olvasva) ===\n");
	for (int i = 0; i < g_nnames; i++) {
		struct dentry de; memset(&de, 0, sizeof(de));
		de.d_sb = dir->i_sb; de.d_parent = root;
		de.d_name.name = (const unsigned char *)g_names[i];
		de.d_name.len = strlen(g_names[i]);
		struct dentry *res = dir->i_op->lookup(dir, &de, 0);
		struct dentry *use = res ? res : &de;
		if (!use->d_inode) { printf("   %s: lookup nem talált inode-ot (res=%p)\n", g_names[i], (void*)res); continue; }
		printf("   [debug] %s: ino=%lu size=%lld mode=%o\n", g_names[i], use->d_inode->i_ino, (long long)use->d_inode->i_size, use->d_inode->i_mode);
		char buf[4096];
		ssize_t n = uk_read(use, buf, sizeof(buf) - 1, 0);
		if (n < 0) { printf("   %s: olvasási hiba\n", g_names[i]); continue; }
		buf[n] = 0;
		printf("   --- %s (%zd bájt) ---\n   %s", g_names[i], n, buf);
		if (n && buf[n-1] != '\n') printf("\n");
	}
	if (argc >= 4 && !strcmp(argv[3], "rw")) {
		printf("=== WRITE-teszt: ukfs_write_file ===\n");
		long w = ukfs_write_file("UKW.TXT", "uKernel ntfs write!\n", 20);
		printf("=== ukfs_write_file = %ld ===\n", w);
	}
	return 0;
}

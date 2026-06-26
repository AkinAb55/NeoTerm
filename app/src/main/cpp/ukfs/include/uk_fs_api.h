/* uKernel — egyszerű C-API a VFS-futásidő és az LD_PRELOAD-bridge (preload_fs.c) között.
 * A preload-lib NEM lát kernel-headereket; csak ezeket a sima POSIX-szignatúrákat hívja. */
#ifndef _UK_FS_API_H
#define _UK_FS_API_H
#include <stddef.h>

struct ukfs_dirent { char name[256]; unsigned long ino; int type; long size; };
/* type: 1=DIR, 2=REG (a DT_* leképezés) */

/* mount: a valódi FS-driver module_init-je + uk_mount; 0=siker */
int  ukfs_mount(const char *fstype, const char *img);
/* remount: a lemez-állapot FRISS újraolvasása (párhuzamos íráshoz, a zár alatt); 0=siker */
int  ukfs_remount(const char *fstype, const char *img);
/* fájlrendszer-statisztika (df / statvfs) a driver s_op->statfs-én át; 0=siker, -1=hiba */
int  ukfs_statfs(unsigned long *bsize, unsigned long long *blocks, unsigned long long *bfree,
                 unsigned long long *bavail, unsigned long long *files, unsigned long long *ffree,
                 long *namelen, long *frsize, long *ftype);
/* a gyökér listázása; visszaad egy belső tömböt + darabszámot */
int  ukfs_list(struct ukfs_dirent **out);
/* egy könyvtár (path; ""=gyökér) listázása — az alkönyvtárakhoz */
int  ukfs_list_dir(const char *path, struct ukfs_dirent **out);
/* egy fájl mérete a gyökérben (-1 ha nincs) */
long ukfs_file_size(const char *name);
/* egy fájl tartalmának olvasása a VALÓDI driverrel; visszaad bájtszámot vagy -1 */
long ukfs_read_file(const char *name, char *buf, size_t maxlen, long pos);
/* létezik-e a gyökér ezen néven (0=könyvtár-gyökér, 1=fájl, -1=nincs) */
int  ukfs_path_kind(const char *name);

/* fájl írása a VALÓDI driverrel (create+write_begin/end); visszaad bájtszámot v. <0 */
long ukfs_write_file(const char *name, const char *data, size_t len);
/* OFFSET-tudatos írás (sparse fájlokhoz): a [offset,offset+len) tartomány írása, a többi LYUK marad;
 * nem vág 0-ra, nem állít méretet, létrehozza a fájlt ha nincs. 0/len=siker, <0=hiba. */
long ukfs_write_file_at(const char *name, const char *data, size_t len, long long offset);
/* a megadott fájl (vagy az egész FS) perzisztálása a block-eszközre — a write-út
 * elhalasztott flush-ének párja, a redirect close-kor küldi (SYNC). 0=siker. */
long ukfs_sync_path(const char *name);
/* alkönyvtár létrehozása a VALÓDI driver i_op->mkdir-jával; 0=siker, <0=hiba */
long ukfs_mkdir(const char *name);
/* special fájl (FIFO/device-node/socket) a driver i_op->mknod-jával; 0=siker, -38=ENOSYS (vfat/exfat) */
long ukfs_mknod(const char *name, unsigned int mode, unsigned long dev);
/* fájl törlése (i_op->unlink) ill. ÜRES alkönyvtár törlése (i_op->rmdir); 0=siker, <0=hiba */
long ukfs_unlink(const char *name);
long ukfs_rmdir(const char *name);
/* átnevezés/áthelyezés (i_op->rename); 0=siker, <0=hiba */
long ukfs_rename(const char *oldpath, const char *newpath);
/* jogosultság ill. tulajdonos módosítása (i_op->setattr); uid/gid==-1 = változatlan; 0=siker */
long ukfs_chmod(const char *name, unsigned int mode);
long ukfs_chown(const char *name, unsigned int uid, unsigned int gid);
/* a VALÓDI inode-attribútumok (mode/uid/gid/size/ino + mtime/atime mp) — a bridge stat()-jához.
 * A *mtime/*atime NULL is lehet (nem kell). 0=siker, -1=nincs. */
int  ukfs_stat(const char *name, unsigned int *mode, unsigned int *uid, unsigned int *gid, long *size, unsigned long *ino, long *mtime, long *atime, unsigned int *nlink, unsigned long *rdev, unsigned long *blocks);
/* fájl-méret állítása (i_op->setattr, ATTR_SIZE); 0=siker, <0=hiba */
long ukfs_truncate(const char *name, long long size);
/* hozzáférési/módosítási idő állítása (i_op->setattr); ansec/mnsec==-1 -> az adott idő OMIT; 0=siker */
long ukfs_utime(const char *name, long asec, long ansec, long msec, long mnsec);
/* szimbolikus link létrehozása (i_op->symlink); 0=siker, <0=hiba (-38=ENOSYS, ha a FS nem támogat) */
long ukfs_symlink(const char *target, const char *linkpath);
/* hard link (i_op->link) — ugyanaz az inode, nlink++; 0=siker, <0=hiba */
long ukfs_link(const char *oldpath, const char *newpath);
/* a symlink célja (i_op->get_link); a bájtszámot adja vissza (buf-ba másolva), vagy <0=hiba */
long ukfs_readlink(const char *path, char *buf, size_t bufsz);

/* kiterjesztett attribútumok (xattr) a VALÓDI driver xattr-handlerein át (ext4/ntfs3; vfat/exfat
 * -EOPNOTSUPP=-95). A name TELJES (prefixes, pl. "user.foo"). flags: XATTR_CREATE=1/REPLACE=2. */
long ukfs_setxattr(const char *path, const char *name, const void *value, size_t size, int flags);
/* az xattr értéke a value-ba (size=0 -> csak a szükséges méretet adja vissza); >=0 méret v. <0 hiba */
long ukfs_getxattr(const char *path, const char *name, void *value, size_t size);
/* az összes xattr-név NUL-terminált listája a list-be (size=0 -> a méret); >=0 hossz v. <0 hiba */
long ukfs_listxattr(const char *path, char *list, size_t size);
/* egy xattr törlése; 0=siker, <0=hiba */
long ukfs_removexattr(const char *path, const char *name);

/* a következő ADAT (is_data=1, SEEK_DATA) ill. LYUK (is_data=0, SEEK_HOLE) pozíciója `offset`-től,
 * a valódi blokk-allokáció alapján (sparse-tudatos cp/tar). >=0 pozíció, -6=ENXIO, -2=ENOENT. */
long long ukfs_seek_data_hole(const char *name, long long offset, int is_data);

/* inode-flag-ek (chattr/lsattr): FS_IMMUTABLE_FL/APPEND_FL/NODUMP_FL... a driver fileattr_get/set-jén
 * át. 0=siker, -95=nem támogatott (vfat/exfat), <0=hiba. */
long ukfs_getflags(const char *name, unsigned int *flags);
long ukfs_setflags(const char *name, unsigned int flags);
/* a fájl extent-térképe (filefrag/FIEMAP): az `arg` a felhasználói struct fiemap*. 0=siker, <0=hiba. */
long ukfs_fiemap(const char *name, void *arg);
#endif

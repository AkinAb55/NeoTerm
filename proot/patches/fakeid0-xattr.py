#!/usr/bin/env python3
# Patch the Termux proot fork's fake_id0 extension to store emulated ownership
# (uid/gid/mode) in a `user.proot.meta` extended attribute on each file, instead
# of `.proot-meta-file.*` sidecar files (USERLAND mode's default) — no rootfs
# clutter. This makes chown persistent and CALLER-INDEPENDENT, so the
# "create as root -> chown user -> start via a root wrapper" pattern (Debian's
# pg_createcluster / pg_ctlcluster, and similar) works under -0 fake root.
#
# Must be paired with -DUSERLAND in the proot build (see build-proot.sh). The
# storage functions are verified on a host with fakeid0-xattr-test/run.sh.
#
# Usage: fakeid0-xattr.py <proot-src-dir>   (the dir containing extension/)

import sys, re
ROOT = sys.argv[1]
def rd(p): return open(p, encoding='utf-8', errors='surrogateescape').read()
def wr(p, s): open(p,'w',encoding='utf-8',errors='surrogateescape').write(s)
def must(c,m):
    if not c: sys.stderr.write("PATCH FAIL: %s\n"%m); sys.exit(9)
FK = ROOT + "/extension/fake_id0/"

# ---- helper_functions.c : xattr storage ----
hf = FK+"helper_functions.c"; s = rd(hf)
must('XATTR_META_NAME' not in s, "helper already patched")
s = s.replace('#define META_TAG ".proot-meta-file."',
  '#include <sys/xattr.h>\n#define META_TAG ".proot-meta-file."\n#define XATTR_META_NAME "user.proot.meta"', 1)
must('XATTR_META_NAME' in s, "insert include/define")

get_meta_new = r'''int get_meta_path(char orig_path[PATH_MAX], char meta_path[PATH_MAX])
{
	/* xattr-backed: the meta lives in an xattr on the file itself, so the
	 * "meta path" is just the real path (no .proot-meta-file sidecars). */
	strncpy(meta_path, orig_path, PATH_MAX - 1);
	meta_path[PATH_MAX - 1] = '\0';
	return 0;
}
'''
s2 = re.sub(r'int get_meta_path\(char orig_path\[PATH_MAX\], char meta_path\[PATH_MAX\]\).*?\n\}\n',
            lambda _m: get_meta_new, s, count=1, flags=re.S)
must(s2!=s, "replace get_meta_path"); s=s2

read_meta_new = r'''int read_meta_file(char path[PATH_MAX], mode_t *mode, uid_t *owner, gid_t *group, Config *config)
{
	char buf[64];
	int lcl_mode;
	ssize_t n = getxattr(path, XATTR_META_NAME, buf, sizeof(buf) - 1);
	if(n <= 0) {
		/* No meta xattr: permissive default (as upstream's missing-meta case). */
		*owner = config->euid;
		*group = config->egid;
		*mode = otod(755);
		return 0;
	}
	buf[n] = '\0';
	if(sscanf(buf, "%d %d %d", &lcl_mode, owner, group) != 3) {
		*owner = config->euid;
		*group = config->egid;
		*mode = otod(755);
		return 0;
	}
	*mode = (mode_t) otod(lcl_mode);
	return 0;
}
'''
s2 = re.sub(r'int read_meta_file\(char path\[PATH_MAX\].*?\n\}\n', lambda _m: read_meta_new, s, count=1, flags=re.S)
must(s2!=s, "replace read_meta_file"); s=s2

write_meta_new = r'''int write_meta_file(char path[PATH_MAX], mode_t mode, uid_t owner, gid_t group,
	bool is_creat, Config *config)
{
	char buf[64];
	int len;
	if(is_creat)
		mode = (mode & ~(config->umask) & 0777);
	len = snprintf(buf, sizeof(buf), "%d\n%d\n%d\n", dtoo(mode), owner, group);
	if(len <= 0 || (size_t) len >= sizeof(buf))
		return -1;
	/* Non-fatal: write_meta is also called at syscall ENTER for mkdir/creat,
	 * when the target does not exist yet (setxattr -> ENOENT), and the fs/SELinux
	 * may reject it. The guest syscall must still succeed; chown persists the
	 * owner later, when the file exists. */
	setxattr(path, XATTR_META_NAME, buf, len, 0);
	return 0;
}

/* xattr-backed meta existence (mirrors path_exists semantics: 0 = present). */
int meta_exists(char path[PATH_MAX])
{
	return (getxattr(path, XATTR_META_NAME, NULL, 0) >= 0) ? 0 : -1;
}
'''
s2 = re.sub(r'int write_meta_file\(char path\[PATH_MAX\].*?\n\}\n', lambda _m: write_meta_new, s, count=1, flags=re.S)
must(s2!=s, "replace write_meta_file"); s=s2
wr(hf, s)

# ---- helper_functions.h : meta_exists prototype ----
hh = FK+"helper_functions.h"; s = rd(hh)
must('int path_exists(char path[PATH_MAX]);' in s, "path_exists proto present")
s = s.replace('int path_exists(char path[PATH_MAX]);',
              'int path_exists(char path[PATH_MAX]);\nint meta_exists(char path[PATH_MAX]);', 1)
wr(hh, s)

# ---- chown.c: drop the "no meta -> skip" early-return so chown ALWAYS persists
#      (by chown time the file exists, so the xattr write in write_meta succeeds) ----
cf=FK+"chown.c"; s=rd(cf)
old_cf="\tif(path_exists(meta_path) != 0)\n\t\treturn 0;\n"
must(old_cf in s, "chown.c early-return block")
s=s.replace(old_cf, "\t/* xattr-backed: always persist ownership on chown. */\n", 1)
wr(cf, s)
# ---- open.c / stat.c: path_exists(meta_path) -> meta_exists(meta_path) ----
for f in ["open.c","stat.c"]:
    p=FK+f; s=rd(p)
    must('path_exists(meta_path)' in s, "%s path_exists(meta_path)"%f)
    s=s.replace('path_exists(meta_path)','meta_exists(meta_path)')
    wr(p,s)

# ---- stat.c : statx handler reads the xattr; add include ----
sp=FK+"stat.c"; s=rd(sp)
if '#include <sys/xattr.h>' not in s:
    s=s.replace('#include "tracee/statx.h"','#include "tracee/statx.h"\n#include <sys/xattr.h>\n#define XATTR_META_NAME "user.proot.meta"',1)
must('XATTR_META_NAME' in s, "stat.c include/define")
statx_new = r'''int fake_id0_handle_statx_syscall(Tracee *tracee, Config *config, uintptr_t statx_state_raw) {
	struct statx_syscall_state *state = (struct statx_syscall_state *) statx_state_raw;
	/* xattr-backed ownership: statx() is what modern glibc stat() uses, so it
	 * must honour the persistent owner stored by chown (user.proot.meta xattr). */
	char path[PATH_MAX];
	if (read_sysarg_path(tracee, path, SYSARG_2, MODIFIED) == 0) {
		char buf[64];
		ssize_t n = getxattr(path, XATTR_META_NAME, buf, sizeof(buf) - 1);
		if (n > 0) {
			int m, o, g;
			buf[n] = '\0';
			if (sscanf(buf, "%d %d %d", &m, &o, &g) == 3) {
				if (state->statx_buf.stx_mask & STATX_UID) {
					state->statx_buf.stx_uid = (uint32_t) o;
					state->updated_stats = true;
				}
				if (state->statx_buf.stx_mask & STATX_GID) {
					state->statx_buf.stx_gid = (uint32_t) g;
					state->updated_stats = true;
				}
				return 0;
			}
		}
	}
	/* Fallback: report the effective uid/gid for app-owned files. */
	if (state->statx_buf.stx_mask & STATX_UID) {
		if (state->statx_buf.stx_uid == getuid()) {
			state->statx_buf.stx_uid = config->euid;
			state->updated_stats = true;
		}
	}
	if (state->statx_buf.stx_mask & STATX_GID) {
		if (state->statx_buf.stx_gid == getgid()) {
			state->statx_buf.stx_gid = config->egid;
			state->updated_stats = true;
		}
	}
	return 0;
}
'''
s2 = re.sub(r'int fake_id0_handle_statx_syscall\(.*?\n\}\n', lambda _m: statx_new, s, count=1, flags=re.S)
must(s2!=s, "replace statx handler"); s=s2
wr(sp, s)
print("ALL PATCHES APPLIED OK")

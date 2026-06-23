#!/usr/bin/env bash
# Host-side verification of the xattr ownership-storage logic that
# fakeid0-xattr.py patches into proot. Downloads the upstream fake_id0 sources,
# applies the patch, extracts the storage functions and runs the PostgreSQL
# ownership scenario (root creates dir -> chown postgres -> both root and
# postgres must see owner=postgres). Requires gcc + a user.*-xattr-capable fs.
set -euo pipefail
HERE=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
PATCH="${HERE}/../fakeid0-xattr.py"
WORK=$(mktemp -d); trap 'rm -rf "$WORK"' EXIT
mkdir -p "$WORK/extension/fake_id0"
base="https://raw.githubusercontent.com/termux/proot/master/src/extension/fake_id0"
for f in helper_functions.c helper_functions.h chown.c open.c stat.c rename.c unlink.c fake_id0.c; do
  curl -fsSL --retry 3 "$base/$f" -o "$WORK/extension/fake_id0/$f"
done
python3 "$PATCH" "$WORK"
# extract the patched storage functions + dtoo/otod into a compilable unit
python3 - "$WORK/extension/fake_id0/helper_functions.c" > "$WORK/patched_funcs.c" <<'PY'
import re,sys
s=open(sys.argv[1],encoding='utf-8',errors='surrogateescape').read()
print('''#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <linux/limits.h>
typedef struct { uid_t ruid, euid, suid; gid_t rgid, egid, sgid; mode_t umask; } Config;
int dtoo(int n){int rem,i=1,octal=0;while(n!=0){rem=n%8;n/=8;octal+=rem*i;i*=10;}return octal;}
int otod(int n){int decimal=0,i=0,rem;while(n!=0){int j,pow=1;for(j=0;j<i;j++)pow*=8;rem=n%10;n/=10;decimal+=rem*pow;++i;}return decimal;}
#define XATTR_META_NAME "user.proot.meta"''')
for sig in [r'int get_meta_path\(',r'int read_meta_file\(',r'int write_meta_file\(',r'int meta_exists\(']:
    print(re.search(sig+r'.*?\n\}\n',s,flags=re.S).group(0))
PY
gcc -O2 -Wall -o "$WORK/ptest" "${HERE}/test.c" "$WORK/patched_funcs.c"
TF=$(mktemp); "$WORK/ptest" "$TF"; rc=$?; rm -f "$TF"
exit $rc

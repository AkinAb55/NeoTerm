#!/usr/bin/env bash
# Host regression tests for the uKernel USB-storage FS stack.
#
# Builds the FS engine + ukfsd for the host (x86_64/glibc) and exercises every
# native layer below proot against a real FAT32 image — the same checks that were
# done by hand while building the feature, now automated:
#
#   1. vfat engine     — direct mount/list/read/write (ukfs_test_vfat)
#   2. ukfsd + sockets — full io.neoterm.fs protocol with the block backend
#                        served over io.neoterm.block (ukfsd_e2e.py)
#   3. proot redirect  — uknl_fs_redirect.c compiles -Wall -Wextra; getdents
#                        synthesis + LIST-blob parsing unit-tested
#   4. proot patch     — fakeid0-xattr.py applies cleanly into a copy of proot
#   5. bionic build    — engine + ukfsd cross-compile/link for aarch64 (if NDK)
#
# Usage:  test/run_host_tests.sh
# Exit 0 iff all runnable tests pass. Tests needing absent tools are skipped.

set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
UKFS="$(cd "$HERE/.." && pwd)"
ROOT="$(cd "$UKFS/../../../../.." && pwd)"   # repo root
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"; for p in ${PIDS:-}; do kill "$p" 2>/dev/null; done' EXIT

PASS=0; FAIL=0; SKIP=0; PIDS=""
ok()   { echo "  PASS  $1"; PASS=$((PASS+1)); }
bad()  { echo "  FAIL  $1"; FAIL=$((FAIL+1)); }
skip() { echo "  SKIP  $1 ($2)"; SKIP=$((SKIP+1)); }
have() { command -v "$1" >/dev/null 2>&1; }

KCF="-O2 -fno-strict-aliasing -fno-builtin -fshort-wchar -D_GNU_SOURCE -D__KERNEL__ -DMODULE -w -Wno-implicit-function-declaration -I $UKFS/include -I $UKFS/linux/fs/fat -I $UKFS/linux/fs/exfat"
FATDEF='-DCONFIG_VFAT_FS=1 -DCONFIG_FAT_FS=1 -DCONFIG_FAT_DEFAULT_CODEPAGE=437 -DCONFIG_FAT_DEFAULT_IOCHARSET="iso8859-1" -DCONFIG_FAT_DEFAULT_UTF8=0'
EXDEF='-DCONFIG_EXFAT_FS=1 -DCONFIG_EXFAT_DEFAULT_IOCHARSET="utf8"'

# ── build the engine + ukfsd + test harness for the host ──────────────────────
build_host() {
  local cc="$1" out="$2"; shift 2; local extra="$*"; local obj=""
  for c in cache dir fatent file inode misc nfs namei_vfat; do
    $cc -c $KCF $FATDEF $extra -DKBUILD_MODNAME="\"v_$c\"" "$UKFS/linux/fs/fat/$c.c" -o "$out/fat_$c.o" 2>"$out/e_$c" || return 1
    obj="$obj $out/fat_$c.o"
  done
  for c in balloc cache dir fatent file inode misc namei nls super; do
    $cc -c $KCF $EXDEF $extra -DKBUILD_MODNAME="\"ex_$c\"" "$UKFS/linux/fs/exfat/$c.c" -o "$out/ex_$c.o" 2>"$out/ee_$c" || { echo "  (build) exfat/$c failed:"; grep -m3 error: "$out/ee_$c"; return 1; }
    obj="$obj $out/ex_$c.o"
  done
  for f in vfs:shim/fs/vfs.c blocksock:shim/fs/block_sock.c posix_acl:shim/fs/posix_acl.c compat:shim/compat_bionic.c; do
    local b="${f%%:*}" src="${f##*:}"
    $cc -c $KCF $FATDEF $extra "$UKFS/$src" -o "$out/$b.o" 2>"$out/e_$b" || { echo "  (build) $src failed:"; grep -m3 error: "$out/e_$b"; return 1; }
    obj="$obj $out/$b.o"
  done
  for src in "$UKFS"/shim/core/*.c; do
    [ "$(basename "$src")" = fileio.c ] && continue
    local b="s_$(basename "$src" .c)"
    $cc -c $KCF $FATDEF $extra "$src" -o "$out/$b.o" 2>/dev/null || return 1
    obj="$obj $out/$b.o"
  done
  echo "$obj" > "$out/.objs"
  return 0
}

echo "== uKernel USB-storage FS — host tests =="
echo "repo: $ROOT"

if ! have gcc; then
  echo "gcc not found — cannot run host tests"; exit 1
fi
mkdir -p "$WORK/o"
echo "[build] compiling FS engine + ukfsd for host ..."
if build_host gcc "$WORK/o"; then
  OBJ="$(cat "$WORK/o/.objs")"
  gcc -o "$WORK/ukfsd"          $OBJ "$UKFS/shim/fs/ukfsd.c"    -I "$UKFS/include" -I "$UKFS/linux/fs/fat" $KCF $FATDEF -lpthread 2>"$WORK/o/lk1" \
    && ok "build ukfsd (host)" || { bad "build ukfsd"; sed -n 1,5p "$WORK/o/lk1"; }
  gcc -o "$WORK/ukfs_test_vfat" $OBJ "$UKFS/shim/fs/ukfs_test.c" -I "$UKFS/include" -I "$UKFS/linux/fs/fat" $KCF $FATDEF -lpthread 2>"$WORK/o/lk2" \
    && ok "build ukfs_test_vfat (host)" || bad "build ukfs_test_vfat"
else
  bad "build FS engine (host)"
fi

# ── make a seeded FAT32 image ─────────────────────────────────────────────────
IMG="$WORK/fat.img"
if have mkfs.vfat && have mcopy; then
  dd if=/dev/zero of="$IMG" bs=1M count=48 status=none 2>/dev/null
  mkfs.vfat -F32 -n UKTEST "$IMG" >/dev/null 2>&1
  printf 'uKernel usb-storage e2e\n' > "$WORK/HELLO.TXT"   # 24 bytes
  mcopy -i "$IMG" "$WORK/HELLO.TXT" ::HELLO.TXT
else
  IMG=""
fi

# ── 1. vfat engine: direct mount/list/read (ukfs_test_vfat) ───────────────────
if [ -x "$WORK/ukfs_test_vfat" ] && [ -n "$IMG" ]; then
  cp "$IMG" "$WORK/eng.img"
  if "$WORK/ukfs_test_vfat" vfat "$WORK/eng.img" 2>/dev/null | grep -q "uKernel usb-storage e2e"; then
    ok "vfat engine: mount + list + read HELLO.TXT"
  else bad "vfat engine: read"; fi
  if "$WORK/ukfs_test_vfat" vfat "$WORK/eng.img" rw 2>/dev/null | grep -q "ukfs_write_file = 20"; then
    ok "vfat engine: write path"
  else bad "vfat engine: write"; fi
else
  skip "vfat engine direct test" "no ukfs_test_vfat or mkfs.vfat/mtools"
fi

# ── 1b. exfat engine: mount + write + read on a fresh image (mtools is FAT-only,
#        so use the harness' rw mode rather than a pre-seeded file) ─────────────
if [ -x "$WORK/ukfs_test_vfat" ] && have mkfs.exfat; then
  dd if=/dev/zero of="$WORK/ex.img" bs=1M count=48 status=none 2>/dev/null
  mkfs.exfat "$WORK/ex.img" >/dev/null 2>&1
  if "$WORK/ukfs_test_vfat" exfat "$WORK/ex.img" rw 2>/dev/null | grep -q "ukfs_write_file = 20"; then
    ok "exfat engine: mount + write + read-back"
  else bad "exfat engine: write/read"; fi
else
  skip "exfat engine direct test" "no ukfs_test_vfat or mkfs.exfat"
fi

# ── 2. ukfsd + io.neoterm.fs/block end-to-end ─────────────────────────────────
if [ -x "$WORK/ukfsd" ] && [ -n "$IMG" ] && have python3; then
  cp "$IMG" "$WORK/e2e.img"
  R=$RANDOM; BSOCK="io.neoterm.block.t$R"; FSOCK="io.neoterm.fs.t$R"
  UKFSD_BLOCKSOCK="$BSOCK" "$WORK/ukfsd" "$FSOCK" >"$WORK/ukfsd.log" 2>&1 &
  PIDS="$PIDS $!"; sleep 0.4
  if timeout 30 python3 "$HERE/ukfsd_e2e.py" "$BSOCK" "$FSOCK" "$WORK/e2e.img" > "$WORK/e2e.out" 2>&1; then
    sed 's/^/    /' "$WORK/e2e.out"; ok "ukfsd e2e: protocol + block-over-socket"
  else
    sed 's/^/    /' "$WORK/e2e.out"; bad "ukfsd e2e"
  fi
else
  skip "ukfsd e2e test" "no ukfsd / mkfs.vfat / python3"
fi

# ── 3. proot VFS-redirect: compile + getdents unit test ───────────────────────
if gcc -Wall -Wextra -Wno-unused-function -Wno-unused-parameter \
       -I "$ROOT/proot/patches" -o "$WORK/redir_test" "$HERE/fs_redirect_test.c" 2>"$WORK/redir.cc"; then
  ok "proot redirect: uknl_fs_redirect.c compiles -Wall -Wextra"
  if "$WORK/redir_test" > "$WORK/redir.out" 2>&1; then
    sed 's/^/    /' "$WORK/redir.out"; ok "proot redirect: getdents synthesis unit test"
  else sed 's/^/    /' "$WORK/redir.out"; bad "proot redirect: getdents unit test"; fi
else
  bad "proot redirect: compile"; sed -n 1,8p "$WORK/redir.cc"
fi

# ── 4. proot patch applies cleanly ────────────────────────────────────────────
if have python3 && [ -d "$ROOT/proot/vendor/proot/src" ]; then
  cp -r "$ROOT/proot/vendor/proot/src" "$WORK/prsrc"
  if python3 "$ROOT/proot/patches/fakeid0-xattr.py" "$WORK/prsrc" >"$WORK/patch.out" 2>&1 \
     && grep -q "ALL PATCHES APPLIED OK" "$WORK/patch.out" \
     && grep -q uknl_fs_dispatch "$WORK/prsrc/syscall/enter.c" \
     && grep -q uknl_fs_open_exit "$WORK/prsrc/syscall/exit.c" \
     && grep -q uk_fs_sysnums "$WORK/prsrc/syscall/seccomp.c"; then
    ok "proot patch applies (enter.c/exit.c/seccomp.c)"
  else bad "proot patch apply"; tail -3 "$WORK/patch.out"; fi
else
  skip "proot patch test" "no python3 / proot src"
fi

# ── 5. bionic cross-compile (optional, needs NDK) ─────────────────────────────
NDK_CC="$(ls /opt/android-ndk-*/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android24-clang 2>/dev/null | head -1)"
if [ -n "${NDK_CC:-}" ]; then
  mkdir -p "$WORK/nb"
  BKCF="-fPIC -Wno-error=implicit-function-declaration -Wno-incompatible-function-pointer-types -Wno-incompatible-pointer-types"
  if build_host "$NDK_CC" "$WORK/nb" "$BKCF" >/dev/null 2>&1; then
    BOBJ="$(cat "$WORK/nb/.objs")"
    "$NDK_CC" $KCF $FATDEF $BKCF -I "$UKFS/include" -I "$UKFS/linux/fs/fat" -o "$WORK/nb/libukfsd.so" $BOBJ "$UKFS/shim/fs/ukfsd.c" 2>"$WORK/nb/lk" \
      && ok "bionic: engine + ukfsd cross-compile/link (aarch64)" || { bad "bionic link"; sed -n 1,4p "$WORK/nb/lk"; }
  else bad "bionic: cross-compile engine"; fi
else
  skip "bionic cross-compile" "Android NDK not installed"
fi

echo
echo "== results: $PASS passed, $FAIL failed, $SKIP skipped =="
[ "$FAIL" -eq 0 ]

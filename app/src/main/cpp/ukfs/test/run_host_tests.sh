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

KCF="-O2 -fno-strict-aliasing -fno-builtin -fshort-wchar -D_GNU_SOURCE -D__KERNEL__ -DMODULE -w -Wno-implicit-function-declaration -I $UKFS/include -I $UKFS/linux/fs/fat -I $UKFS/linux/fs/exfat -I $UKFS/linux/fs/ntfs3 -I $UKFS/linux/fs/ntfs3/lib -I $UKFS/linux/fs/ext4 -I $UKFS/linux/fs/jbd2"
FATDEF='-DCONFIG_VFAT_FS=1 -DCONFIG_FAT_FS=1 -DCONFIG_FAT_DEFAULT_CODEPAGE=437 -DCONFIG_FAT_DEFAULT_IOCHARSET="iso8859-1" -DCONFIG_FAT_DEFAULT_UTF8=0'
EXDEF='-DCONFIG_EXFAT_FS=1 -DCONFIG_EXFAT_DEFAULT_IOCHARSET="utf8"'
NDEF='-DCONFIG_NTFS3_FS=1 -DCONFIG_NTFS3_LZX_XPRESS=1 -DCONFIG_NLS_DEFAULT="utf8"'
E4DEF='-DCONFIG_EXT4_FS=1 -DCONFIG_JBD2=1'

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
  for c in "$UKFS"/linux/fs/ntfs3/*.c; do
    local b="$(basename "$c" .c)"
    $cc -c $KCF $NDEF $extra -DKBUILD_MODNAME="\"n3_$b\"" "$c" -o "$out/n3_$b.o" 2>"$out/ne_$b" || { echo "  (build) ntfs3/$b failed:"; grep -m3 error: "$out/ne_$b"; return 1; }
    obj="$obj $out/n3_$b.o"
  done
  for c in "$UKFS"/linux/fs/ntfs3/lib/*.c; do
    local b="$(basename "$c" .c)"
    $cc -c $KCF $NDEF $extra -include linux/minmax.h -DKBUILD_MODNAME="\"n3lib_$b\"" "$c" -o "$out/n3lib_$b.o" 2>"$out/nle_$b" || { echo "  (build) ntfs3/lib/$b failed:"; grep -m3 error: "$out/nle_$b"; return 1; }
    obj="$obj $out/n3lib_$b.o"
  done
  for c in "$UKFS"/linux/fs/jbd2/*.c "$UKFS"/linux/fs/ext4/*.c; do
    local b="$(basename "$c" .c)"
    $cc -c $KCF $E4DEF $extra -DKBUILD_MODNAME="\"e4_$b\"" "$c" -o "$out/e4_$b.o" 2>"$out/e4e_$b" || { echo "  (build) ext4/$b failed:"; grep -m3 error: "$out/e4e_$b"; return 1; }
    obj="$obj $out/e4_$b.o"
  done
  for f in vfs:shim/fs/vfs.c blocksock:shim/fs/block_sock.c posix_acl:shim/fs/posix_acl.c ntstub:shim/fs/ntfs3_stubs.c e4stub:shim/fs/ext4_stubs.c compat:shim/compat_bionic.c; do
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

# ── 1c. ntfs3 engine: mount + write + read-back on a fresh image ───────────────
if [ -x "$WORK/ukfs_test_vfat" ] && have mkfs.ntfs; then
  dd if=/dev/zero of="$WORK/nt.img" bs=1M count=48 status=none 2>/dev/null
  mkfs.ntfs -F -Q "$WORK/nt.img" >/dev/null 2>&1
  if "$WORK/ukfs_test_vfat" ntfs3 "$WORK/nt.img" rw 2>/dev/null | grep -q "ukfs_write_file = 20"; then
    ok "ntfs3 engine: mount + write + read-back"
  else bad "ntfs3 engine: write/read"; fi
else
  skip "ntfs3 engine direct test" "no ukfs_test_vfat or mkfs.ntfs"
fi

# ── 1d. ext4 engine: mount + write + read-back on a fresh image ────────────────
if [ -x "$WORK/ukfs_test_vfat" ] && have mkfs.ext4; then
  dd if=/dev/zero of="$WORK/e4.img" bs=1M count=48 status=none 2>/dev/null
  mkfs.ext4 -q -F "$WORK/e4.img" >/dev/null 2>&1
  if "$WORK/ukfs_test_vfat" ext4 "$WORK/e4.img" rw 2>/dev/null | grep -q "ukfs_write_file = 20"; then
    ok "ext4 engine: mount + write + read-back"
  else bad "ext4 engine: write/read"; fi
else
  skip "ext4 engine direct test" "no ukfs_test_vfat or mkfs.ext4"
fi

# ── 1e. partition table: PARTS enumeration + per-partition MOUNT (uksd0pN) ─────
#        Build a Raspberry-Pi-like MBR image (FAT32 boot + ext4 root) hermetically
#        — mkfs into files, dd into the whole-disk image at the partition offsets,
#        hand-write the MBR table (no loop device / root needed), seed FAT via mcopy. ──
if [ -x "$WORK/ukfsd" ] && have mkfs.vfat && have mkfs.ext4 && have mcopy && have python3; then
  PDIR="$WORK/pdev"; mkdir -p "$PDIR"
  P1_START=2048; P1_SECS=$((48*1024*1024/512))           # 48 MiB FAT32 @ 1 MiB
  P2_START=$((P1_START+P1_SECS)); P2_SECS=$((64*1024*1024/512))  # 64 MiB ext4
  TOTAL=$((P2_START+P2_SECS+2048))
  dd if=/dev/zero of="$PDIR/uksd0" bs=512 count="$TOTAL" status=none
  dd if=/dev/zero of="$WORK/p1.fat" bs=512 count="$P1_SECS" status=none
  mkfs.vfat -F32 -n BOOT "$WORK/p1.fat" >/dev/null 2>&1
  printf 'console=serial0 root=/dev/mmcblk0p2\n' > "$WORK/cmdline.txt"
  MTOOLS_SKIP_CHECK=1 mcopy -i "$WORK/p1.fat" "$WORK/cmdline.txt" ::CMDLINE.TXT 2>/dev/null
  dd if=/dev/zero of="$WORK/p2.ext4" bs=512 count="$P2_SECS" status=none
  mkfs.ext4 -q -F -L rootfs "$WORK/p2.ext4" >/dev/null 2>&1
  python3 - "$PDIR/uksd0" "$WORK/p1.fat" "$WORK/p2.ext4" "$P1_START" "$P1_SECS" "$P2_START" "$P2_SECS" <<'PY'
import struct, sys
img,p1,p2,s1,n1,s2,n2 = sys.argv[1], sys.argv[2], sys.argv[3], *map(int, sys.argv[4:8])
with open(img,'r+b') as o:
    with open(p1,'rb') as f: o.seek(s1*512); o.write(f.read())
    with open(p2,'rb') as f: o.seek(s2*512); o.write(f.read())
    def ent(boot,t,st,sc): return struct.pack('<B3sB3sII',boot,b'\xfe\xff\xff',t,b'\xfe\xff\xff',st,sc)
    o.seek(0x1BE); o.write(ent(0x80,0x0C,s1,n1)); o.write(ent(0x00,0x83,s2,n2)); o.write(b'\0'*32)
    o.seek(0x1FE); o.write(b'\x55\xaa')
PY
  R=$RANDOM; PFSOCK="io.neoterm.fs.p$R"
  UKFSD_DEVDIR="$PDIR/" "$WORK/ukfsd" "$PFSOCK" >"$WORK/ukfsd_parts.log" 2>&1 &
  PIDS="$PIDS $!"; sleep 0.4
  if timeout 40 python3 "$HERE/ukfsd_parts.py" "$PFSOCK" > "$WORK/parts.out" 2>&1; then
    sed 's/^/    /' "$WORK/parts.out"; ok "partition table: PARTS + mount uksd0p1 (FAT) + uksd0p2 (ext4)"
  else
    sed 's/^/    /' "$WORK/parts.out"; bad "partition table support"
  fi
else
  skip "partition table test" "no ukfsd / mkfs.vfat / mkfs.ext4 / mcopy / python3"
fi

# ── 1f. fused: FUSE "kernel" engine vs a REAL libfuse3 daemon ─────────────────
#        proot/patches/fused.c speaks the kernel side of the FUSE protocol
#        (vendored UAPI fuse_kernel.h) so guest libfuse daemons (sshfs/rclone/…)
#        work over our /dev/fuse. Validate it against an in-memory rw filesystem
#        built on real libfuse3, driven over a socketpair via custom_io. ──
if have cc && { pkg-config --exists fuse3 2>/dev/null || [ -f /usr/include/fuse3/fuse.h ]; }; then
  FCF="$(pkg-config --cflags fuse3 2>/dev/null || true)"
  FLB="$(pkg-config --libs fuse3 2>/dev/null || echo -lfuse3)"
  if cc -Wall -Wextra -Wno-unused-parameter -I "$ROOT/proot/patches" $FCF \
        "$HERE/fused_hello_test.c" "$ROOT/proot/patches/fused.c" -o "$WORK/fused_test" $FLB 2>"$WORK/o/fused.cc"; then
    if timeout 30 "$WORK/fused_test" >"$WORK/fused.out" 2>/dev/null && grep -q "all checks passed" "$WORK/fused.out"; then
      sed 's/^/    /' "$WORK/fused.out"
      ok "fused: FUSE engine vs real libfuse3 (init/getattr/readdir/open/read/create/write/mkdir/truncate/unlink)"
    else sed 's/^/    /' "$WORK/fused.out"; bad "fused: libfuse3 round-trip"; fi
  else bad "fused: build vs libfuse3"; sed -n 1,6p "$WORK/o/fused.cc"; fi
else
  skip "fused FUSE engine test" "no cc / libfuse3-dev"
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
       -I "$ROOT/proot/patches" -o "$WORK/redir_test" "$HERE/fs_redirect_test.c" "$ROOT/proot/patches/fused.c" 2>"$WORK/redir.cc"; then
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

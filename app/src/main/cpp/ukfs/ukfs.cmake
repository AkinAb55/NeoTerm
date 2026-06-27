# uKernel FS engine — Android/aarch64 (bionic) cross-build.
#
# Runs the REAL Linux kernel FS drivers (linux/fs/*) in userspace against
# uKernel's fake kernel headers (include/) + the core kernel-API shim
# (shim/core) + the VFS bridge (shim/fs/vfs.c). This file builds the
# standalone mount+list test (ukfs_test_vfat) to validate the cross-compile;
# the ukfsd socket server is layered on top once this is green.
#
# Mirrors build/build_ukfs.sh from the uKernel project (host gcc) but for the
# NDK toolchain. See docs/USB_STORAGE_MOUNT.md.

set(UKFS_DIR ${CMAKE_CURRENT_LIST_DIR})
set(UKFS_INC ${UKFS_DIR}/include)

# Common flags for the kernel-side translation units (drivers + vfs + shim).
# -D__KERNEL__/-DMODULE: the fake headers present the in-kernel API.
# -fshort-wchar: the kernel treats wchar_t as 16-bit (FAT/exFAT/NTFS name tables).
# -fno-builtin/-fno-strict-aliasing: kernel code assumes these.
set(UKFS_KCFLAGS
  -fPIC -O2 -fno-strict-aliasing -fno-builtin -fshort-wchar -D_GNU_SOURCE
  -D__KERNEL__ -DMODULE
  # clang (NDK) promotes these kernel-isms to hard errors; the original gcc
  # build left them as warnings. The kernel relies on implicit decls resolving
  # at link and on loose function-pointer signatures.
  -Wno-implicit-function-declaration -Wno-error=implicit-function-declaration
  -Wno-incompatible-pointer-types -Wno-incompatible-function-pointer-types
  -Wno-unused -Wno-unused-parameter -Wno-sign-compare
  -Wno-implicit-fallthrough -Wno-missing-braces -Wno-unknown-pragmas)

# --- core kernel-API shim (kmalloc/printk/kmem_cache/mutex/module_inits/...) ---
file(GLOB UKFS_SHIM_CORE ${UKFS_DIR}/shim/core/*.c)
# fileio.c pulls usb/net glue we don't need for the FS path; drop it.
list(FILTER UKFS_SHIM_CORE EXCLUDE REGEX "/fileio\\.c$")

# --- vfat driver: only the vfat namei (msdos namei module-init would clash) ---
set(UKFS_FAT_SRC cache dir fatent file inode misc nfs namei_vfat)
set(UKFS_FAT_DEFS
  -DCONFIG_VFAT_FS=1 -DCONFIG_FAT_FS=1 -DCONFIG_FAT_DEFAULT_CODEPAGE=437
  -DCONFIG_FAT_DEFAULT_IOCHARSET="iso8859-1" -DCONFIG_FAT_DEFAULT_UTF8=0)

set(UKFS_OBJ_SRCS "")
foreach(c ${UKFS_FAT_SRC})
  list(APPEND UKFS_OBJ_SRCS ${UKFS_DIR}/linux/fs/fat/${c}.c)
  # Each driver file needs a UNIQUE KBUILD_MODNAME so the fat-core init
  # (inode cache) and the vfat init (register_filesystem) land in separate
  # module slots and BOTH run.
  set_source_files_properties(${UKFS_DIR}/linux/fs/fat/${c}.c PROPERTIES
    COMPILE_DEFINITIONS "KBUILD_MODNAME=\"vfat_${c}\"")
endforeach()

# --- exfat driver: the full driver (its own super.c registers exfat_fs_type).
#     FS-agnostic shim (vfs.c) + the same VFS bridge serve it; the MOUNT
#     auto-probe in ukfsd tries vfat then exfat then ntfs3/ext4. ---
set(UKFS_EXFAT_SRC balloc cache dir fatent file inode misc namei nls super)
set(UKFS_EXFAT_DEFS -DCONFIG_EXFAT_FS=1 -DCONFIG_EXFAT_DEFAULT_IOCHARSET="utf8")
foreach(c ${UKFS_EXFAT_SRC})
  list(APPEND UKFS_OBJ_SRCS ${UKFS_DIR}/linux/fs/exfat/${c}.c)
  # exfat needs its CONFIG_* (per-source, since the engine target carries the
  # vfat defs) plus a unique KBUILD_MODNAME per file.
  set_source_files_properties(${UKFS_DIR}/linux/fs/exfat/${c}.c PROPERTIES
    COMPILE_DEFINITIONS "KBUILD_MODNAME=\"exfat_${c}\";CONFIG_EXFAT_FS=1;CONFIG_EXFAT_DEFAULT_IOCHARSET=\"utf8\"")
endforeach()

# --- ntfs3 driver: the full in-tree driver (its super.c registers ntfs3_fs_type).
#     Larger than fat/exfat (uses an rbtree for run maps -> shim/core/rbtree.c, and a
#     lib/ LZX/XPRESS decompressor). ntfs3_stubs.c supplies the few extra kernel
#     symbols the shim doesn't. The MOUNT auto-probe tries it after vfat/exfat. ---
set(UKFS_NTFS_DEFS -DCONFIG_NTFS3_FS=1 -DCONFIG_NTFS3_LZX_XPRESS=1 -DCONFIG_NLS_DEFAULT="utf8")
file(GLOB UKFS_NTFS_MAIN ${UKFS_DIR}/linux/fs/ntfs3/*.c)
foreach(c ${UKFS_NTFS_MAIN})
  get_filename_component(b ${c} NAME_WE)
  list(APPEND UKFS_OBJ_SRCS ${c})
  set_source_files_properties(${c} PROPERTIES
    COMPILE_DEFINITIONS "KBUILD_MODNAME=\"ntfs3_${b}\";CONFIG_NTFS3_FS=1;CONFIG_NTFS3_LZX_XPRESS=1;CONFIG_NLS_DEFAULT=\"utf8\"")
endforeach()
file(GLOB UKFS_NTFS_LIB ${UKFS_DIR}/linux/fs/ntfs3/lib/*.c)
foreach(c ${UKFS_NTFS_LIB})
  get_filename_component(b ${c} NAME_WE)
  list(APPEND UKFS_OBJ_SRCS ${c})
  # the LZX decompressor uses swap() without pulling <linux/minmax.h>; force-include it.
  set_source_files_properties(${c} PROPERTIES
    COMPILE_DEFINITIONS "KBUILD_MODNAME=\"ntfs3lib_${b}\";CONFIG_NTFS3_FS=1"
    COMPILE_OPTIONS "-include;linux/minmax.h")
endforeach()

# --- shared FS engine: the vfat driver + VFS/ACL shim + kernel-API shim,
#     compiled once and linked into both the test harness and the ukfsd server. ---
add_library(ukfs_engine OBJECT
  ${UKFS_DIR}/shim/fs/vfs.c
  ${UKFS_DIR}/shim/fs/block_sock.c   # block-over-socket backend (io.neoterm.block)
  ${UKFS_DIR}/shim/fs/posix_acl.c
  ${UKFS_DIR}/shim/fs/ntfs3_stubs.c  # extra kernel symbols ntfs3 needs
  ${UKFS_DIR}/shim/compat_bionic.c   # backtrace/hex/system_wq/get_random_u32 shims
  ${UKFS_OBJ_SRCS}
  ${UKFS_SHIM_CORE})
target_include_directories(ukfs_engine PRIVATE ${UKFS_INC} ${UKFS_DIR}/linux/fs/fat ${UKFS_DIR}/linux/fs/exfat ${UKFS_DIR}/linux/fs/ntfs3 ${UKFS_DIR}/linux/fs/ntfs3/lib)
target_compile_options(ukfs_engine PRIVATE ${UKFS_KCFLAGS} ${UKFS_FAT_DEFS})

# --- ukfsd: io.neoterm.fs unix-socket server (the Android FS daemon) ---
# Output as libukfsd.so (a PIE executable) so AGP packages it into
# jniLibs/<abi>/ and Android extracts it, executable, to nativeLibraryDir —
# the same deployment trick as libproot.so. FsBridge.kt launches it from there.
add_executable(ukfsd ${UKFS_DIR}/shim/fs/ukfsd.c $<TARGET_OBJECTS:ukfs_engine>)
set_target_properties(ukfsd PROPERTIES PREFIX "lib" OUTPUT_NAME "ukfsd" SUFFIX ".so")
target_include_directories(ukfsd PRIVATE ${UKFS_INC} ${UKFS_DIR}/linux/fs/fat ${UKFS_DIR}/linux/fs/exfat ${UKFS_DIR}/linux/fs/ntfs3 ${UKFS_DIR}/linux/fs/ntfs3/lib)
target_compile_options(ukfsd PRIVATE ${UKFS_KCFLAGS} ${UKFS_FAT_DEFS})
target_link_libraries(ukfsd dl)

# --- ukfs_test_vfat: standalone mount/list/read/write harness (dev/debug) ---
add_executable(ukfs_test_vfat ${UKFS_DIR}/shim/fs/ukfs_test.c $<TARGET_OBJECTS:ukfs_engine>)
target_include_directories(ukfs_test_vfat PRIVATE ${UKFS_INC} ${UKFS_DIR}/linux/fs/fat ${UKFS_DIR}/linux/fs/exfat ${UKFS_DIR}/linux/fs/ntfs3 ${UKFS_DIR}/linux/fs/ntfs3/lib)
target_compile_options(ukfs_test_vfat PRIVATE ${UKFS_KCFLAGS} ${UKFS_FAT_DEFS})
target_link_libraries(ukfs_test_vfat dl)

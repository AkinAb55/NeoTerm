package io.neoterm.setup.usbserial

import android.system.Os
import io.neoterm.component.config.NeoPreference
import io.neoterm.component.config.NeoTermPath
import io.neoterm.setup.proot.Kmsg
import java.io.File

/**
 * Fake `/sys/block` + `/sys/dev/block` tree for the USB pendrive, so the
 * standard Linux block tooling enumerates /dev/uksd0 — `lsblk`, `rpi-imager`,
 * `gnome-disks`, `gparted`, `blkid -o list`, … all walk sysfs to find drives, and
 * Android's SELinux blocks the real /sys/dev/block (EACCES). Same idea as
 * [SensorBridge]'s /sys/class/power_supply and /sys/bus/iio binds.
 *
 * The trees are real host dirs bound onto the guest by ProotManager. They exist
 * (empty) from proot launch; [refresh] fills them when a device attaches (size +
 * partition table read via [BlockBridge]), [clear] empties them on unplug — the
 * binds are static so live writes appear in the guest immediately.
 *
 * Device-number convention matches the block proxy's stat (rdev 8:0, sd-like):
 * the whole disk is 8:0, partition N is 8:N.
 */
object BlockSysfsBridge {
  private const val TAG = "usb-block-sysfs"
  private val blockDir = File("${NeoTermPath.PROOT_ROOT_PATH}/sys-block")
  private val devBlockDir = File("${NeoTermPath.PROOT_ROOT_PATH}/sys-dev-block")

  private data class Part(val idx: Int, val startBytes: Long, val sizeBytes: Long)

  /** Public partition descriptor (idx, byte offset, byte size). */
  data class PartInfo(val idx: Int, val startBytes: Long, val sizeBytes: Long)

  /** Parse the device's partition table (MBR primary + GPT) via [read]. Shared with
   *  [BlockBridge] so the block proxy can expose /dev/uksd0pN nodes at the right
   *  offsets. Empty list = no/invalid table (whole-device "superfloppy"). */
  fun partitions(read: (Long, Int) -> ByteArray?): List<PartInfo> =
    runCatching { parsePartitions(read).map { PartInfo(it.idx, it.startBytes, it.sizeBytes) } }
      .getOrDefault(emptyList())

  /** Host dir → guest path binds (empty list when USB storage is off). */
  fun sysfsBinds(): List<Pair<String, String>> {
    if (!NeoPreference.isUsbStorageEnabled()) return emptyList()
    blockDir.mkdirs(); devBlockDir.mkdirs()      // bound even when empty (no drive yet)
    return listOf(
      blockDir.absolutePath to "/sys/block",
      devBlockDir.absolutePath to "/sys/dev/block"
    )
  }

  private fun w(dir: File, name: String, value: String) {
    val f = File(dir, name)
    f.parentFile?.mkdirs()
    runCatching { f.writeText(value + "\n") }
  }

  private fun symlink(link: File, target: String) {
    runCatching { if (link.exists() || isSymlink(link)) link.delete() }
    runCatching { Os.symlink(target, link.absolutePath) }
  }

  private fun isSymlink(f: File): Boolean = runCatching { Os.lstat(f.absolutePath).st_mode and 0xF000 == 0xA000 }.getOrDefault(false)

  /** Wipe the trees (device unplugged). */
  @Synchronized
  fun clear() {
    runCatching { blockDir.deleteRecursively() }
    runCatching { devBlockDir.deleteRecursively() }
    blockDir.mkdirs(); devBlockDir.mkdirs()
  }

  /**
   * (Re)build the sysfs tree for an attached device of [totalBytes] bytes with the
   * given [sectorSize]; [read] reads raw bytes off the device (for the partition
   * table). Safe to call repeatedly.
   */
  @Synchronized
  fun refresh(totalBytes: Long, sectorSize: Int, read: (Long, Int) -> ByteArray?) {
    if (!NeoPreference.isUsbStorageEnabled() || totalBytes <= 0L) { clear(); return }
    clear()
    val disk = File(blockDir, "uksd0").apply { mkdirs() }
    val sect512 = totalBytes / 512
    w(disk, "dev", "8:0")
    w(disk, "size", sect512.toString())          // /sys size is ALWAYS in 512-byte units
    w(disk, "removable", "1")
    w(disk, "ro", "0")
    w(disk, "range", "16")
    w(disk, "alignment_offset", "0")
    w(disk, "discard_alignment", "0")
    w(disk, "hidden", "0")
    w(disk, "capability", "51")
    w(disk, "queue/logical_block_size", sectorSize.toString())
    w(disk, "queue/physical_block_size", sectorSize.toString())
    w(disk, "queue/hw_sector_size", sectorSize.toString())
    w(disk, "queue/minimum_io_size", sectorSize.toString())
    w(disk, "queue/optimal_io_size", "0")
    w(disk, "queue/rotational", "0")
    w(disk, "queue/nr_requests", "128")
    w(disk, "queue/read_ahead_kb", "128")
    w(disk, "queue/max_sectors_kb", "1280")
    w(disk, "queue/scheduler", "[none]")
    w(disk, "queue/add_random", "0")
    w(disk, "device/model", "NeoTerm USB")
    w(disk, "device/vendor", "NeoTerm ")
    symlink(File(devBlockDir, "8:0"), "../../block/uksd0")

    val parts = runCatching { parsePartitions(read) }.getOrDefault(emptyList())
    for (p in parts) {
      val pdir = File(disk, "uksd0p${p.idx}").apply { mkdirs() }
      w(pdir, "dev", "8:${p.idx}")
      w(pdir, "partition", p.idx.toString())
      w(pdir, "start", (p.startBytes / 512).toString())
      w(pdir, "size", (p.sizeBytes / 512).toString())
      w(pdir, "ro", "0")
      w(pdir, "alignment_offset", "0")
      w(pdir, "discard_alignment", "0")
      symlink(File(devBlockDir, "8:${p.idx}"), "../../block/uksd0/uksd0p${p.idx}")
    }
    Kmsg.log("usb-block-sysfs: /sys/block/uksd0 (${sect512 * 512} B, ${parts.size} partitions)")
  }

  // ── partition table parse (MBR primary + GPT) — mirrors ukfs_probe_partitions ──
  private fun u32(b: ByteArray, o: Int): Long =
    (b[o].toLong() and 0xff) or ((b[o + 1].toLong() and 0xff) shl 8) or
    ((b[o + 2].toLong() and 0xff) shl 16) or ((b[o + 3].toLong() and 0xff) shl 24)
  private fun u64(b: ByteArray, o: Int): Long = u32(b, o) or (u32(b, o + 4) shl 32)

  private fun parsePartitions(read: (Long, Int) -> ByteArray?): List<Part> {
    val mbr = read(0, 512) ?: return emptyList()
    if (mbr.size < 512 || (mbr[510].toInt() and 0xff) != 0x55 || (mbr[511].toInt() and 0xff) != 0xAA)
      return emptyList()
    // GPT protective MBR?
    if ((mbr[0x1BE + 4].toInt() and 0xff) == 0xEE) return parseGpt(read)
    val out = ArrayList<Part>()
    for (i in 0 until 4) {
      val e = 0x1BE + i * 16
      val type = mbr[e + 4].toInt() and 0xff
      val start = u32(mbr, e + 8); val secs = u32(mbr, e + 12)
      if (type == 0 || secs == 0L) continue
      if (type == 0x05 || type == 0x0F) continue   // extended: skip (logical not exposed in sysfs)
      out.add(Part(i + 1, start * 512L, secs * 512L))
    }
    return out
  }

  private fun parseGpt(read: (Long, Int) -> ByteArray?): List<Part> {
    val hdr = read(512, 512) ?: return emptyList()
    if (hdr.size < 92 || String(hdr, 0, 8, Charsets.US_ASCII) != "EFI PART") return emptyList()
    val entLba = u64(hdr, 72); val num = u32(hdr, 80).toInt(); val esz = u32(hdr, 84).toInt()
    if (esz < 128 || esz > 4096 || num <= 0 || num > 256) return emptyList()
    val out = ArrayList<Part>()
    for (i in 0 until num) {
      if (out.size >= 64) break
      val ent = read(entLba * 512 + i.toLong() * esz, esz) ?: break
      if (ent.size < 56) break
      var empty = true
      for (k in 0 until 16) if (ent[k].toInt() != 0) { empty = false; break }
      if (empty) continue
      val first = u64(ent, 32); val last = u64(ent, 40)
      if (last < first) continue
      out.add(Part(i + 1, first * 512L, (last - first + 1) * 512L))
    }
    return out
  }
}

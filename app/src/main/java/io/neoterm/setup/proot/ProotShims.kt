package io.neoterm.setup.proot

import android.system.Os
import io.neoterm.utils.NLog
import java.io.File

/**
 * Guest-oldali kompatibilitási shimek, amiket a rootfs `/usr/local/bin`-jébe
 * írunk (a PATH-ban megelőzi a `/usr/bin`-t, így leárnyékolja a disztró
 * eszközeit, anélkül hogy felülírná őket). Minden indításkor frissítjük
 * ([install]), így a meglévő telepítéseknél is megjelennek.
 *
 * - `systemctl`: proot-ban nincs systemd (PID 1) → a `start/stop/status/…`
 *   igéket SysV initre (`service` / `/etc/init.d`) fordítja, a boot/daemon
 *   igéket sikeres no-opként kezeli. Így a Debian/Kali csomagscriptek és a
 *   `msfdb` nem halnak el a „System has not been booted with systemd" hibán.
 * - `dmesg`: Android tiltja a kernel ring buffert → egy rövid, valószerű
 *   boot-logot ad a NeoTerm/proot környezetről (hibázás helyett).
 * - `journalctl` / `loginctl`: minimális no-op stubok (sok script hívja őket).
 */
object ProotShims {

  private const val SYSTEMCTL = """#!/bin/sh
# Minimal systemctl shim proot-hoz (nincs systemd / PID 1). A gyakori igéket
# SysV initre (service / /etc/init.d) fordítja, a boot/daemon igéket sikeres
# no-opként kezeli. NeoTerm.
case "${'$'}1" in --version|-v|version) echo "systemd 0 (systemctl-shim for proot)"; exit 0;; esac

verb=""; unit=""
for a in "${'$'}@"; do
  case "${'$'}a" in
    -*) : ;;                                  # flagek (--now, --no-pager, -f) eldobva
    *) if [ -z "${'$'}verb" ]; then verb="${'$'}a"; elif [ -z "${'$'}unit" ]; then unit="${'$'}{a%.service}"; fi ;;
  esac
done

svc() {                                       # service -> init.d fallback
  if command -v service >/dev/null 2>&1; then service "${'$'}unit" "${'$'}1"
  elif [ -x "/etc/init.d/${'$'}unit" ]; then "/etc/init.d/${'$'}unit" "${'$'}1"
  else echo "systemctl-shim: nincs init script ehhez: '${'$'}unit'" >&2; return 1; fi
}

case "${'$'}verb" in
  start|stop|restart|reload|force-reload|try-restart) if [ -n "${'$'}unit" ]; then svc "${'$'}verb"; else exit 0; fi ;;
  status)            if [ -n "${'$'}unit" ]; then svc status; else exit 0; fi ;;
  is-active)         if [ -n "${'$'}unit" ] && svc status >/dev/null 2>&1; then echo active; else echo inactive; exit 3; fi ;;
  enable|disable)    command -v update-rc.d >/dev/null 2>&1 && update-rc.d "${'$'}unit" "${'$'}verb" >/dev/null 2>&1; exit 0 ;;
  is-enabled)        echo enabled; exit 0 ;;
  daemon-reload|daemon-reexec|reset-failed|preset|preset-all|set-default|mask|unmask) exit 0 ;;
  is-system-running) echo running; exit 0 ;;
  list-units|list-unit-files|show|cat) exit 0 ;;
  *)                 exit 0 ;;                 # ismeretlen ige -> csendes siker
esac
"""

  private const val DMESG = """#!/bin/sh
# Minimal dmesg proot-hoz: Android tiltja a kernel ring buffert, ezért egy
# rövid, valószerű boot-logot adunk a NeoTerm/proot userlandről. NeoTerm.
emit() { printf '[%11.6f] %s\n' "${'$'}2" "${'$'}1"; }
ver=${'$'}(cat /proc/version 2>/dev/null || echo "Linux")
up=${'$'}(cut -d' ' -f1 /proc/uptime 2>/dev/null || echo 0)
emit "${'$'}ver" 0.000000
emit "NeoTerm proot userland: fake root (-0), USERLAND xattr ownership" 0.000100
emit "Command line: BOOT_IMAGE=proot quiet" 0.000200
mnt=${'$'}(awk '{print ${'$'}2}' /proc/mounts 2>/dev/null | tr '\n' ' ')
[ -n "${'$'}mnt" ] && emit "Mounts: ${'$'}mnt" 0.000300
emit "NeoTerm runtime (uptime): ${'$'}{up}s" "${'$'}up"
"""

  private const val NOOP = "#!/bin/sh\nexit 0\n"

  /** Kiírja/frissíti a shimeket a rootfs `/usr/local/bin`-jébe (idempotens). */
  fun install(rootfs: String) {
    val binDir = File(rootfs, "usr/local/bin")
    if (!binDir.isDirectory && !binDir.mkdirs()) {
      NLog.e("ProotShims", "Cannot create $binDir")
      return
    }
    write(binDir, "systemctl", SYSTEMCTL)
    write(binDir, "dmesg", DMESG)
    write(binDir, "journalctl", NOOP)
    write(binDir, "loginctl", NOOP)
  }

  private fun write(dir: File, name: String, content: String) {
    runCatching {
      val f = File(dir, name)
      f.writeText(content)
      Os.chmod(f.absolutePath, 493 /* 0755 */)
    }.onFailure {
      NLog.e("ProotShims", "Cannot write shim $name: ${it.message}")
    }
  }
}

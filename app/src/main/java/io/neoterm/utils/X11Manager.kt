package io.neoterm.utils

import android.content.Context
import android.content.Intent
import android.os.Build
import com.termux.x11.MainActivity
import com.termux.x11.NeoX11Service
import io.neoterm.component.config.NeoTermPath
import io.neoterm.setup.proot.ProotManager
import java.io.File

/**
 * Drives the embedded Termux:X11 native X server, built into the NeoTerm APK
 * (the :x11 module — single APK, no separate package).
 *
 * The X server runs in its OWN process via [NeoX11Service]
 * (android:process=":x11server"), mirroring how Termux:X11 runs its server in a
 * dedicated process. This keeps NeoTerm's main thread free (no UI freeze) and,
 * being an AMS-managed foreground service rather than a forked child process,
 * survives Android 12+ phantom-process killing.
 *
 * [launchDisplay] opens `com.termux.x11.MainActivity` (the LorieView surface) in
 * the normal app process; it connects to the server cross-process over the
 * ACTION_START binder. proot sessions already export DISPLAY=:0.
 *
 * @author kiva
 */
object X11Manager {
  private const val ACTIVITY = "com.termux.x11.MainActivity"

  /** Always true now: the X server ships inside the NeoTerm APK. */
  fun isServerInstalled(context: Context): Boolean = true

  /** Launch the X11 display activity (the window that shows the X server output). */
  fun launchDisplay(context: Context) {
    runCatching {
      context.startActivity(
        Intent().setClassName(context.packageName, ACTIVITY)
          .addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
      )
    }.onFailure {
      NLog.e("X11Manager", "Failed to open X11 display: ${it.localizedMessage}")
    }
  }

  /**
   * Start the X server (in its own process) on display :0, then open the
   * display. The server creates the abstract X socket that proot apps reach via
   * DISPLAY=:0, and broadcasts ACTION_START to our package so MainActivity
   * connects.
   *
   * The native server reads its config from the environment, which is
   * per-process, so we compute the values here (in the app, where the distro
   * paths are known) and hand them to the service to apply in the server
   * process:
   *  - TMPDIR: the server creates its socket at $TMPDIR/.X11-unix/X0; we point
   *    it at the dir proot binds to the guest's /tmp/.X11-unix (see ProotManager).
   *  - XKB_CONFIG_ROOT: keyboard config data the server requires, from the
   *    selected distro's rootfs (/usr/share/X11/xkb).
   */
  fun startServer(context: Context) {
    runCatching {
      val tmp = File(NeoTermPath.PROOT_ROOT_PATH, "x11")
      File(tmp, ".X11-unix").mkdirs()
      val xkb = File(ProotManager.selectedDistro().rootfsPath(), "usr/share/X11/xkb")
      if (!xkb.exists()) {
        NLog.e(
          "X11Manager",
          "XKB data missing at ${xkb.absolutePath} — run 'Install X11 environment' first"
        )
      }

      val intent = Intent(context, NeoX11Service::class.java)
        .putExtra(NeoX11Service.EXTRA_TMPDIR, tmp.absolutePath)
        .putExtra(NeoX11Service.EXTRA_XKB, xkb.absolutePath)
      if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
        context.startForegroundService(intent)
      } else {
        context.startService(intent)
      }
      NLog.e("X11Manager", "Requested X server (:x11server) on :0")
    }.onFailure {
      NLog.e("X11Manager", "Failed to start X server: ${it.localizedMessage}")
    }
    launchDisplay(context)
  }

  /**
   * Stop the X server: close the GUI window and stop the :x11server service,
   * whose onDestroy kills that process so the native server actually exits.
   */
  fun stopServer(context: Context) {
    runCatching {
      MainActivity.getInstance()?.finishAndRemoveTask()
      context.stopService(Intent(context, NeoX11Service::class.java))
      NLog.e("X11Manager", "Requested X server stop")
    }.onFailure {
      NLog.e("X11Manager", "Failed to stop X server: ${it.localizedMessage}")
    }
  }
}

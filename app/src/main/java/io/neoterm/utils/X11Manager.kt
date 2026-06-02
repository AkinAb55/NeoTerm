package io.neoterm.utils

import android.content.Context
import android.content.Intent
import android.os.Handler
import android.os.Looper
import android.system.Os
import com.termux.x11.CmdEntryPoint
import io.neoterm.component.config.NeoTermPath
import io.neoterm.setup.proot.ProotManager
import java.io.File

/**
 * Drives the embedded Termux:X11 native X server, which is built directly into
 * the NeoTerm APK (the :x11 module — single APK, no separate package).
 *
 * The X server runs **in NeoTerm's own process** (via [CmdEntryPoint.startInProcess]),
 * not as a child app_process: Android 12+ kills app-spawned child processes as
 * "phantom processes" (SIGKILL) once the app backgrounds, which killed the X
 * server instantly. In-process, the server is protected by NeoTerm's foreground
 * service and libXlorie.so loads normally from nativeLibraryDir.
 *
 * [launchDisplay] opens `com.termux.x11.MainActivity` (the LorieView surface),
 * which lives in the same process and connects to the server over the local
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
   * Start the X server in-process on display :0, then open the display. The
   * server creates the abstract X socket that proot apps reach via DISPLAY=:0,
   * and broadcasts ACTION_START to our own package so the embedded MainActivity
   * connects. Server bring-up must happen on the main thread.
   */
  fun startServer(context: Context) {
    val app = context.applicationContext
    prepareServerEnv()
    // Open the GUI first; MainActivity registers the ACTION_START receiver, and
    // the server re-broadcasts every second until connected, so order is safe.
    launchDisplay(context)
    Handler(Looper.getMainLooper()).post {
      runCatching {
        System.loadLibrary("Xlorie")
        CmdEntryPoint.startInProcess(app, arrayOf(":0"))
        NLog.e("X11Manager", "In-process X server requested on :0")
      }.onFailure {
        NLog.e("X11Manager", "Failed to start in-process X server: ${it.localizedMessage}")
      }
    }
  }

  /**
   * Set up the process env the native X server reads (getenv):
   *  - TMPDIR: the server creates its unix socket at $TMPDIR/.X11-unix/X0. We
   *    point it at the dir proot binds to the guest's /tmp/.X11-unix (see
   *    ProotManager), so DISPLAY=:0 inside the distro reaches the same socket.
   *  - XKB_CONFIG_ROOT: the keyboard config data (/usr/share/X11/xkb) the server
   *    requires to start — it lives in the selected distro's rootfs once the X11
   *    environment (xkb-data / xkeyboard-config) is installed.
   */
  private fun prepareServerEnv() {
    runCatching {
      val tmp = File(NeoTermPath.PROOT_ROOT_PATH, "x11")
      File(tmp, ".X11-unix").mkdirs()
      Os.setenv("TMPDIR", tmp.absolutePath, true)

      val xkb = File(ProotManager.selectedDistro().rootfsPath(), "usr/share/X11/xkb")
      if (xkb.exists()) {
        Os.setenv("XKB_CONFIG_ROOT", xkb.absolutePath, true)
      } else {
        NLog.e(
          "X11Manager",
          "XKB data missing at ${xkb.absolutePath} — run 'Install X11 environment' first"
        )
      }
    }.onFailure {
      NLog.e("X11Manager", "Failed to prepare X11 env: ${it.localizedMessage}")
    }
  }
}

package io.neoterm.utils

import android.app.Activity
import android.app.DownloadManager
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.net.Uri
import androidx.core.content.FileProvider
import io.neoterm.component.config.NeoTermPath
import java.io.File

/**
 * Drives the embedded Termux:X11 native X server: installs it (downloaded from
 * our own release, so the user doesn't have to fetch it manually), launches its
 * display activity, and starts the X server bound to display :0. proot sessions
 * already export DISPLAY=:0, so GUI apps installed in the distro connect to it.
 *
 * @author kiva
 */
object X11Manager {
  const val PACKAGE = "com.termux.x11"
  private const val ACTIVITY = "com.termux.x11.MainActivity"
  private const val CMD_ENTRY = "com.termux.x11.CmdEntryPoint"
  private const val APK_NAME = "termux-x11.apk"
  private val APK_URL = "${NeoTermPath.DEFAULT_PROOT_SOURCE}/$APK_NAME"

  fun isServerInstalled(context: Context): Boolean {
    return try {
      context.packageManager.getPackageInfo(PACKAGE, 0)
      true
    } catch (e: Exception) {
      false
    }
  }

  /** Launch the X11 display activity (the window that shows the X server output). */
  fun launchDisplay(context: Context) {
    runCatching {
      context.startActivity(
        Intent().setClassName(PACKAGE, ACTIVITY).addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
      )
    }
  }

  /**
   * Start the X server (CmdEntryPoint) as a host process bound to :0, then show
   * the display. The server creates the abstract X socket that proot apps reach
   * via DISPLAY=:0. Mirrors how Termux launches Termux:X11.
   */
  fun startServer(context: Context) {
    runCatching {
      val apk = context.packageManager.getApplicationInfo(PACKAGE, 0).sourceDir
      val builder = ProcessBuilder(
        "/system/bin/app_process",
        "-Djava.class.path=$apk",
        "/system/bin",
        CMD_ENTRY,
        ":0"
      )
      builder.environment()["CLASSPATH"] = apk
      builder.redirectErrorStream(true)
      builder.start()
    }.onFailure {
      NLog.e("X11Manager", "Failed to start X server: ${it.localizedMessage}")
    }
    launchDisplay(context)
  }

  /** Download the bundled Termux:X11 APK from our release and launch the installer. */
  fun downloadAndInstall(activity: Activity) {
    val target = File(activity.getExternalFilesDir(null), APK_NAME)
    if (target.exists()) target.delete()

    val manager = activity.getSystemService(Context.DOWNLOAD_SERVICE) as DownloadManager
    val request = DownloadManager.Request(Uri.parse(APK_URL))
      .setTitle("Termux:X11")
      .setMimeType("application/vnd.android.package-archive")
      .setNotificationVisibility(DownloadManager.Request.VISIBILITY_VISIBLE_NOTIFY_COMPLETED)
      .setDestinationInExternalFilesDir(activity, null, APK_NAME)
    val downloadId = manager.enqueue(request)

    val appContext = activity.applicationContext
    val receiver = object : BroadcastReceiver() {
      override fun onReceive(context: Context, intent: Intent) {
        if (intent.getLongExtra(DownloadManager.EXTRA_DOWNLOAD_ID, -1) != downloadId) return
        runCatching { appContext.unregisterReceiver(this) }
        installApk(appContext, target)
      }
    }
    appContext.registerReceiver(receiver, IntentFilter(DownloadManager.ACTION_DOWNLOAD_COMPLETE))
  }

  private fun installApk(context: Context, apk: File) {
    if (!apk.exists()) return
    runCatching {
      val uri = FileProvider.getUriForFile(context, "${context.packageName}.fileprovider", apk)
      context.startActivity(
        Intent(Intent.ACTION_VIEW)
          .setDataAndType(uri, "application/vnd.android.package-archive")
          .addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION or Intent.FLAG_ACTIVITY_NEW_TASK)
      )
    }.onFailure {
      NLog.e("X11Manager", "Cannot launch installer: ${it.localizedMessage}")
    }
  }
}

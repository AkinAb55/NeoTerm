package io.neoterm.utils

import android.app.Activity
import android.app.DownloadManager
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.net.Uri
import android.os.Build
import android.os.Handler
import android.os.Looper
import android.provider.Settings
import androidx.core.content.FileProvider
import io.neoterm.BuildConfig
import org.json.JSONObject
import java.io.File
import java.net.HttpURLConnection
import java.net.URL

/**
 * In-app updater based on GitHub Releases: checks the latest release, compares
 * its version with the installed one, downloads the APK and launches the
 * system installer (requesting the install-unknown-apps permission as needed).
 *
 * @author kiva
 */
object UpdateManager {
  private const val REPO = "9hm2/NeoTerm-pr"
  private const val RELEASES_API = "https://api.github.com/repos/$REPO/releases/latest"
  private const val RELEASES_PAGE = "https://github.com/$REPO/releases/latest"
  private const val UPDATE_APK_NAME = "neoterm-update.apk"

  private val mainHandler = Handler(Looper.getMainLooper())

  data class UpdateInfo(
    val versionName: String,
    val tag: String,
    val apkUrl: String?,
    val notes: String
  )

  fun releasesPageUrl(): String = RELEASES_PAGE

  /**
   * Check GitHub for a newer release on a background thread. [onResult] runs on
   * the main thread with the update info, or null if already up to date / on error.
   */
  fun checkForUpdate(onResult: (UpdateInfo?) -> Unit) {
    Thread {
      val info = try {
        fetchLatest()
      } catch (e: Exception) {
        NLog.e("UpdateManager", "Update check failed: ${e.localizedMessage}")
        null
      }
      val newer = if (info != null && compareVersions(info.versionName, BuildConfig.VERSION_NAME) > 0) info else null
      mainHandler.post { onResult(newer) }
    }.start()
  }

  private fun fetchLatest(): UpdateInfo? {
    val connection = URL(RELEASES_API).openConnection() as HttpURLConnection
    connection.connectTimeout = 10000
    connection.readTimeout = 15000
    connection.instanceFollowRedirects = true
    connection.setRequestProperty("Accept", "application/vnd.github+json")
    connection.setRequestProperty("User-Agent", "NeoTerm")
    if (connection.responseCode !in 200..299) {
      return null
    }
    val body = connection.inputStream.bufferedReader().use { it.readText() }
    val json = JSONObject(body)
    val tag = json.optString("tag_name")
    if (tag.isEmpty()) return null
    val notes = json.optString("body")
    val version = tag.trimStart('v', 'V')

    var apkUrl: String? = null
    val assets = json.optJSONArray("assets")
    if (assets != null) {
      for (i in 0 until assets.length()) {
        val asset = assets.getJSONObject(i)
        val name = asset.optString("name")
        if (!name.endsWith(".apk", ignoreCase = true)) continue
        val url = asset.optString("browser_download_url")
        if (apkUrl == null) apkUrl = url
        // Prefer an arm64 / release APK when several are published.
        if (name.contains("arm64", true) || name.contains("release", true)) {
          apkUrl = url
          break
        }
      }
    }
    return UpdateInfo(version, tag, apkUrl, notes)
  }

  /** Compare dotted version strings as integer components. >0 if [a] is newer than [b]. */
  private fun compareVersions(a: String, b: String): Int {
    val pa = a.split(Regex("[._\\-+]")).mapNotNull { it.toIntOrNull() }
    val pb = b.split(Regex("[._\\-+]")).mapNotNull { it.toIntOrNull() }
    for (i in 0 until maxOf(pa.size, pb.size)) {
      val x = pa.getOrElse(i) { 0 }
      val y = pb.getOrElse(i) { 0 }
      if (x != y) return x - y
    }
    return 0
  }

  /** True if the app may install packages (always pre-O; otherwise needs the grant). */
  fun canInstall(context: Context): Boolean {
    return Build.VERSION.SDK_INT < Build.VERSION_CODES.O ||
      context.packageManager.canRequestPackageInstalls()
  }

  /** Send the user to the system screen to allow installing unknown apps. */
  fun requestInstallPermission(activity: Activity) {
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
      runCatching {
        activity.startActivity(
          Intent(Settings.ACTION_MANAGE_UNKNOWN_APP_SOURCES, Uri.parse("package:${activity.packageName}"))
        )
      }
    }
  }

  /**
   * Download the update APK via DownloadManager (showing a download notification)
   * and launch the installer once it finishes. Falls back to the releases page
   * if the release has no APK asset.
   */
  fun downloadAndInstall(activity: Activity, info: UpdateInfo) {
    val url = info.apkUrl
    if (url.isNullOrEmpty()) {
      openReleasesPage(activity)
      return
    }

    val target = File(activity.getExternalFilesDir(null), UPDATE_APK_NAME)
    if (target.exists()) target.delete()

    val manager = activity.getSystemService(Context.DOWNLOAD_SERVICE) as DownloadManager
    val request = DownloadManager.Request(Uri.parse(url))
      .setTitle("NeoTerm ${info.tag}")
      .setMimeType("application/vnd.android.package-archive")
      .setNotificationVisibility(DownloadManager.Request.VISIBILITY_VISIBLE_NOTIFY_COMPLETED)
      .setDestinationInExternalFilesDir(activity, null, UPDATE_APK_NAME)
    val downloadId = manager.enqueue(request)

    val appContext = activity.applicationContext
    val receiver = object : BroadcastReceiver() {
      override fun onReceive(context: Context, intent: Intent) {
        val finished = intent.getLongExtra(DownloadManager.EXTRA_DOWNLOAD_ID, -1)
        if (finished != downloadId) return
        runCatching { appContext.unregisterReceiver(this) }
        installApk(appContext, target)
      }
    }
    appContext.registerReceiver(receiver, IntentFilter(DownloadManager.ACTION_DOWNLOAD_COMPLETE))
  }

  private fun installApk(context: Context, apk: File) {
    if (!apk.exists()) {
      NLog.e("UpdateManager", "Downloaded APK is missing")
      return
    }
    runCatching {
      val uri = FileProvider.getUriForFile(context, "${context.packageName}.fileprovider", apk)
      val intent = Intent(Intent.ACTION_VIEW)
        .setDataAndType(uri, "application/vnd.android.package-archive")
        .addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION or Intent.FLAG_ACTIVITY_NEW_TASK)
      context.startActivity(intent)
    }.onFailure {
      NLog.e("UpdateManager", "Cannot launch installer: ${it.localizedMessage}")
    }
  }

  fun openReleasesPage(context: Context) {
    runCatching {
      context.startActivity(
        Intent(Intent.ACTION_VIEW, Uri.parse(RELEASES_PAGE)).addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
      )
    }
  }
}

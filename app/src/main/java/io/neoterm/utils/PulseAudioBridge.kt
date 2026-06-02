package io.neoterm.utils

import android.content.Context
import android.media.AudioAttributes
import android.media.AudioFormat
import android.media.AudioTrack
import android.system.Os
import io.neoterm.BuildConfig
import org.apache.commons.compress.archivers.tar.TarArchiveInputStream
import org.apache.commons.compress.compressors.gzip.GzipCompressorInputStream
import java.io.File
import java.io.FileInputStream
import java.io.InputStream

/**
 * Android-side PulseAudio: plays the distro's audio on the device speaker.
 *
 * A vanilla PulseAudio cross-built for Android (bundled in assets as
 * pulseaudio-aarch64.tar.gz) runs as the app uid — no root, no proot, so it's
 * stable — configured with:
 *   - module-native-protocol-tcp on :4713 — distro apps connect via
 *     PULSE_SERVER=127.0.0.1:4713 (set in ProotManager);
 *   - module-pipe-sink → a FIFO we read here and write to an AudioTrack.
 *
 * @author kiva
 */
object PulseAudioBridge {
  private const val SAMPLE_RATE = 48000

  @Volatile private var running = false
  private var thread: Thread? = null
  private var paProcess: Process? = null

  fun start(context: Context) {
    if (running) return
    running = true
    val app = context.applicationContext
    val dir = prepare(app)
    if (dir == null) {
      running = false
      return
    }
    val fifo = File(dir, "runtime/fifo")
    startPulseAudio(dir, fifo)
    thread = Thread({ pumpLoop(fifo) }, "pulse-bridge").apply { isDaemon = true; start() }
  }

  fun stop() {
    running = false
    thread?.interrupt()
    thread = null
    runCatching { paProcess?.destroy() }
    paProcess = null
  }

  /** Extract the bundled PulseAudio once per app version; returns its dir. */
  private fun prepare(context: Context): File? {
    return runCatching {
      val dir = File(context.filesDir, "pulseaudio")
      val verFile = File(dir, ".ver")
      val ver = BuildConfig.VERSION_CODE.toString()
      val bin = File(dir, "bin/pulseaudio")
      if (bin.exists() && verFile.exists() && verFile.readText() == ver) {
        return@runCatching dir
      }
      dir.deleteRecursively()
      dir.mkdirs()
      context.assets.open("pulseaudio-aarch64.tar.gz").use { raw ->
        GzipCompressorInputStream(raw).use { gz ->
          TarArchiveInputStream(gz).use { tar ->
            var entry = tar.nextTarEntry
            while (entry != null) {
              val out = File(dir, entry.name)
              if (entry.isDirectory) {
                out.mkdirs()
              } else {
                out.parentFile?.mkdirs()
                out.outputStream().use { tar.copyTo(it) }
              }
              entry = tar.nextTarEntry
            }
          }
        }
      }
      bin.setExecutable(true, false)
      File(dir, "runtime").mkdirs()
      verFile.writeText(ver)
      dir
    }.onFailure {
      NLog.e("PulseAudioBridge", "Failed to extract PulseAudio: ${it.localizedMessage}")
    }.getOrNull()
  }

  private fun startPulseAudio(dir: File, fifo: File) {
    runCatching {
      val runtime = File(dir, "runtime").apply { mkdirs() }
      if (fifo.exists()) fifo.delete()
      runCatching { Os.mkfifo(fifo.absolutePath, 432 /* 0660 */) }

      val bin = File(dir, "bin/pulseaudio").absolutePath
      val modules = File(dir, "lib/pulseaudio/modules").absolutePath
      val log = File(dir, "pulse.log").absolutePath
      val pb = ProcessBuilder(
        bin, "-n", "--daemonize=no", "--exit-idle-time=-1", "--disable-shm=true",
        "--dl-search-path=$modules",
        "--log-target=newfile:$log", "--log-level=notice",
        "-L", "module-native-protocol-tcp port=4713 auth-ip-acl=127.0.0.1 auth-anonymous=1",
        "-L", "module-pipe-sink file=${fifo.absolutePath} sink_name=neoterm rate=$SAMPLE_RATE channels=2 format=s16le"
      )
      val env = pb.environment()
      env["HOME"] = runtime.absolutePath
      env["XDG_RUNTIME_DIR"] = runtime.absolutePath
      env["PULSE_RUNTIME_PATH"] = runtime.absolutePath
      env["LD_LIBRARY_PATH"] = File(dir, "lib").absolutePath
      pb.redirectErrorStream(true)
      pb.redirectOutput(ProcessBuilder.Redirect.to(File(dir, "pulse-stdout.log")))
      paProcess = pb.start()
      NLog.e("PulseAudioBridge", "PulseAudio started on :4713")
    }.onFailure {
      NLog.e("PulseAudioBridge", "Failed to start PulseAudio: ${it.localizedMessage}")
    }
  }

  private fun pumpLoop(fifo: File) {
    val minBuf = AudioTrack.getMinBufferSize(
      SAMPLE_RATE, AudioFormat.CHANNEL_OUT_STEREO, AudioFormat.ENCODING_PCM_16BIT
    )
    val bufSize = maxOf(minBuf, SAMPLE_RATE)
    while (running) {
      var track: AudioTrack? = null
      var input: InputStream? = null
      try {
        // Blocks until PulseAudio's pipe-sink opens the write end.
        input = FileInputStream(fifo)
        track = AudioTrack.Builder()
          .setAudioAttributes(
            AudioAttributes.Builder()
              .setUsage(AudioAttributes.USAGE_MEDIA)
              .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
              .build()
          )
          .setAudioFormat(
            AudioFormat.Builder()
              .setSampleRate(SAMPLE_RATE)
              .setEncoding(AudioFormat.ENCODING_PCM_16BIT)
              .setChannelMask(AudioFormat.CHANNEL_OUT_STEREO)
              .build()
          )
          .setBufferSizeInBytes(bufSize)
          .setTransferMode(AudioTrack.MODE_STREAM)
          .build()
        track.play()

        val buf = ByteArray(8192)
        while (running) {
          val n = input.read(buf)
          if (n < 0) break
          if (n > 0) track.write(buf, 0, n)
        }
      } catch (e: Exception) {
        // FIFO not ready yet / stream dropped — retry.
      } finally {
        runCatching { track?.stop() }
        runCatching { track?.release() }
        runCatching { input?.close() }
      }
      if (running) {
        try {
          Thread.sleep(800)
        } catch (e: InterruptedException) {
          break
        }
      }
    }
  }
}

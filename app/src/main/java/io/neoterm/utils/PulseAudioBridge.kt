package io.neoterm.utils

import android.media.AudioAttributes
import android.media.AudioFormat
import android.media.AudioTrack
import io.neoterm.setup.proot.ProotManager
import java.io.File
import java.net.InetSocketAddress
import java.net.Socket

/**
 * Plays the distro's audio on the Android speaker.
 *
 * We run PulseAudio inside the proot distro (started here in --system mode so it
 * works as root) configured with:
 *   - module-native-protocol-tcp on :4713 — distro apps connect via
 *     PULSE_SERVER=127.0.0.1:4713 (set in ProotManager);
 *   - a null sink whose monitor is streamed as raw PCM over
 *     module-simple-protocol-tcp on :4712.
 *
 * This bridge is the TCP client of that :4712 stream: it reads s16le/48k/stereo
 * PCM and writes it to an AudioTrack. It retries until PulseAudio is up, so the
 * order of bring-up doesn't matter.
 *
 * @author kiva
 */
object PulseAudioBridge {
  private const val PCM_PORT = 4712
  private const val SAMPLE_RATE = 48000

  @Volatile private var running = false
  private var thread: Thread? = null
  private var paProcess: Process? = null

  fun start() {
    if (running) return
    running = true
    startDistroPulseAudio()
    thread = Thread({ pumpLoop() }, "pulse-bridge").apply { isDaemon = true; start() }
  }

  fun stop() {
    running = false
    thread?.interrupt()
    thread = null
    runCatching { paProcess?.destroy() }
    paProcess = null
  }

  private fun pumpLoop() {
    val minBuf = AudioTrack.getMinBufferSize(
      SAMPLE_RATE, AudioFormat.CHANNEL_OUT_STEREO, AudioFormat.ENCODING_PCM_16BIT
    )
    val bufSize = maxOf(minBuf, SAMPLE_RATE) // ~quarter second of stereo s16
    while (running) {
      var socket: Socket? = null
      var track: AudioTrack? = null
      try {
        socket = Socket().apply {
          tcpNoDelay = true
          connect(InetSocketAddress("127.0.0.1", PCM_PORT), 1500)
        }
        val input = socket.getInputStream()
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
        // PulseAudio not up yet, or the stream dropped — retry below.
      } finally {
        runCatching { track?.stop() }
        runCatching { track?.release() }
        runCatching { socket?.close() }
      }
      if (running) {
        try {
          Thread.sleep(1000)
        } catch (e: InterruptedException) {
          break
        }
      }
    }
  }

  /** Write the PA config into the distro and start the daemon (idempotent). */
  private fun startDistroPulseAudio() {
    runCatching {
      val cfg = "/root/.config/pulse/neoterm.pa"
      val script = buildString {
        append("mkdir -p /root/.config/pulse; ")
        append("cat > ").append(cfg).append(" <<'EOF'\n")
        append("load-module module-native-protocol-tcp port=4713 auth-ip-acl=127.0.0.1 auth-anonymous=1\n")
        append("load-module module-null-sink sink_name=neoterm sink_properties=device.description=NeoTerm rate=48000 channels=2\n")
        append("set-default-sink neoterm\n")
        append("load-module module-simple-protocol-tcp source=neoterm.monitor record=true format=s16le rate=48000 channels=2 listen=127.0.0.1 port=4712\n")
        append("EOF\n")
        // Run in the FOREGROUND (not --daemonize): the daemon needs this proot
        // alive for path translation, so we keep the proot process running and
        // tear it down on stop(). --system runs it as root in the container.
        append("exec pulseaudio --system --disallow-exit --disable-shm --exit-idle-time=-1 ")
        append("-nF ").append(cfg).append(" --daemonize=no ")
        append("--log-target=stderr\n")
      }
      val launch = ProotManager.buildLaunch(command = listOf(script))
      val pb = ProcessBuilder(mutableListOf(launch.executable).apply { addAll(launch.args) })
      val env = pb.environment()
      launch.env.forEach {
        val i = it.indexOf('=')
        if (i >= 0) env[it.substring(0, i)] = it.substring(i + 1)
      }
      pb.directory(File(launch.hostCwd))
      pb.redirectErrorStream(true)
      // Capture output so PulseAudio failures are diagnosable: cat this file.
      val log = File(io.neoterm.App.get().filesDir, "x11/pulse.log")
      runCatching { log.parentFile?.mkdirs() }
      pb.redirectOutput(ProcessBuilder.Redirect.to(log))
      paProcess = pb.start()
    }.onFailure {
      NLog.e("PulseAudioBridge", "Failed to start distro PulseAudio: ${it.localizedMessage}")
    }
  }
}

package io.neoterm.utils

import android.Manifest
import android.content.Context
import android.content.pm.PackageManager
import android.location.GnssStatus
import android.location.Location
import android.location.LocationListener
import android.location.LocationManager
import android.location.OnNmeaMessageListener
import android.os.Handler
import android.os.HandlerThread
import android.os.SystemClock
import android.util.Log
import androidx.core.content.ContextCompat
import io.neoterm.component.config.NeoPreference
import java.io.OutputStream
import java.net.InetAddress
import java.net.ServerSocket
import java.net.Socket
import java.util.Locale
import java.util.concurrent.CopyOnWriteArraySet
import kotlin.math.abs

/**
 * Android-side GPS bridge: feeds the device's GNSS data to the distro as an NMEA 0183 stream that
 * gpsd consumes, the same way [PulseAudioBridge] exposes audio.
 *
 * A TCP server (app uid, holding ACCESS_FINE_LOCATION) emits NMEA on 127.0.0.1:[PORT]. In the
 * distro, point gpsd at it:
 *     gpsd "$NEOTERM_GPS_NMEA"      # = tcp://127.0.0.1:4716
 * then the usual clients (cgps, gpspipe, gpsmon, foxtrotgps, …) read gpsd on :2947.
 *
 * Completeness: we forward the GNSS chip's *raw* NMEA verbatim (via [OnNmeaMessageListener]) — all
 * sentence types (RMC/GGA/GSA/GSV/VTG/GST/ZDA), every constellation, real DOP/SNR. On devices that
 * don't deliver raw NMEA through the listener we fall back to synthesizing RMC/GGA/GSA/GSV from
 * [Location] + [GnssStatus] so gpsd still gets a full fix and sky view.
 *
 * GNSS is powered lazily — only while a client (gpsd) is connected — so location isn't sampled
 * when nothing reads it. Started with the app by NeoTermService (proot only).
 */
object GpsBridge {
  private const val TAG = "GpsBridge"
  private const val PORT = 4716
  /** If no raw NMEA arrives within this window while we have a fix, synthesize it instead. */
  private const val RAW_NMEA_GRACE_MS = 4000L
  private const val SYNTH_INTERVAL_MS = 1000L

  @Volatile private var running = false
  private var serverThread: Thread? = null
  private var serverSocket: ServerSocket? = null
  private var appContext: Context? = null

  private val clients = CopyOnWriteArraySet<OutputStream>()

  // Location state (guarded by locationLock).
  private val locationLock = Any()
  private var locationThread: HandlerThread? = null
  private var locationHandler: Handler? = null
  private var locationStarted = false
  private var nmeaListener: OnNmeaMessageListener? = null
  private var locationListener: LocationListener? = null
  private var gnssCallback: GnssStatus.Callback? = null

  @Volatile private var lastRawNmeaAt = 0L
  @Volatile private var latestLocation: Location? = null
  @Volatile private var latestGnss: GnssStatus? = null

  fun start(context: Context) {
    if (running) return
    if (!NeoPreference.isGpsEnabled()) return
    if (ContextCompat.checkSelfPermission(context, Manifest.permission.ACCESS_FINE_LOCATION)
      != PackageManager.PERMISSION_GRANTED
    ) return
    running = true
    appContext = context.applicationContext
    serverThread = Thread({ serverLoop() }, "gps-bridge").apply { isDaemon = true; start() }
  }

  fun stop() {
    running = false
    runCatching { serverSocket?.close() }
    serverSocket = null
    serverThread = null
    clients.clear()
    stopLocation()
  }

  /** Restart after the location permission is granted or the toggle changes. */
  fun restart(context: Context) {
    stop()
    start(context)
  }

  private fun serverLoop() {
    try {
      ServerSocket(PORT, 4, InetAddress.getByName("127.0.0.1")).use { server ->
        serverSocket = server
        while (running) {
          val client = try {
            server.accept()
          } catch (e: Exception) {
            if (running) Log.w(TAG, "accept failed", e)
            break
          }
          Thread({ handleClient(client) }, "gps-client").apply { isDaemon = true }.start()
        }
      }
    } catch (e: Exception) {
      Log.w(TAG, "gps server stopped", e)
    } finally {
      serverSocket = null
    }
  }

  private fun handleClient(socket: Socket) {
    val out = try { socket.getOutputStream() } catch (e: Exception) { return }
    clients.add(out)
    if (clients.size == 1) startLocation()
    try {
      runCatching { socket.tcpNoDelay = true }
      // gpsd just reads the NMEA we push; block on the input until it disconnects.
      val input = socket.getInputStream()
      val buf = ByteArray(256)
      while (running && !socket.isClosed) {
        val n = input.read(buf)
        if (n < 0) break
      }
    } catch (e: Exception) {
      // Client went away — normal.
    } finally {
      clients.remove(out)
      runCatching { socket.close() }
      if (clients.isEmpty()) stopLocation()
    }
  }

  /** Send one NMEA sentence (without trailing newline) to every connected client. */
  private fun broadcast(sentence: String) {
    if (clients.isEmpty()) return
    val bytes = (sentence + "\r\n").toByteArray()
    for (out in clients) {
      try {
        synchronized(out) { out.write(bytes); out.flush() }
      } catch (e: Exception) {
        clients.remove(out)
      }
    }
  }

  // ---- Location / NMEA sources ----

  @android.annotation.SuppressLint("MissingPermission") // guarded by checkSelfPermission in start()
  private fun startLocation() {
    synchronized(locationLock) {
      if (locationStarted) return
      val ctx = appContext ?: return
      if (ContextCompat.checkSelfPermission(ctx, Manifest.permission.ACCESS_FINE_LOCATION)
        != PackageManager.PERMISSION_GRANTED
      ) return
      try {
        val lm = ctx.getSystemService(Context.LOCATION_SERVICE) as LocationManager
        val thread = HandlerThread("gps-capture").apply { start() }
        val handler = Handler(thread.looper)
        locationThread = thread
        locationHandler = handler

        // Raw NMEA straight from the GNSS chip — the most complete source.
        val nmea = OnNmeaMessageListener { message, _ ->
          if (message != null) {
            lastRawNmeaAt = SystemClock.elapsedRealtime()
            broadcast(message.trim().trimEnd('\r', '\n'))
          }
        }
        nmeaListener = nmea
        lm.addNmeaListener(nmea, handler)

        // Satellite status, used for the synthesized GSV/GSA fallback.
        val gnss = object : GnssStatus.Callback() {
          override fun onSatelliteStatusChanged(status: GnssStatus) {
            latestGnss = status
          }
        }
        gnssCallback = gnss
        lm.registerGnssStatusCallback(gnss, handler)

        // Power the GNSS engine.
        val listener = LocationListener { loc -> latestLocation = loc }
        locationListener = listener
        if (lm.allProviders.contains(LocationManager.GPS_PROVIDER)) {
          lm.requestLocationUpdates(LocationManager.GPS_PROVIDER, 1000L, 0f, listener, thread.looper)
        }

        // Synthesized-NMEA fallback loop (only emits when raw NMEA isn't flowing).
        handler.postDelayed(object : Runnable {
          override fun run() {
            if (!locationStarted) return
            emitSynthIfNeeded()
            locationHandler?.postDelayed(this, SYNTH_INTERVAL_MS)
          }
        }, SYNTH_INTERVAL_MS)

        locationStarted = true
      } catch (e: Exception) {
        Log.w(TAG, "startLocation failed", e)
        stopLocation()
      }
    }
  }

  private fun stopLocation() {
    synchronized(locationLock) {
      val ctx = appContext
      if (ctx != null) {
        runCatching {
          val lm = ctx.getSystemService(Context.LOCATION_SERVICE) as LocationManager
          nmeaListener?.let { lm.removeNmeaListener(it) }
          gnssCallback?.let { lm.unregisterGnssStatusCallback(it) }
          locationListener?.let { lm.removeUpdates(it) }
        }
      }
      nmeaListener = null
      gnssCallback = null
      locationListener = null
      runCatching { locationThread?.quitSafely() }
      locationThread = null
      locationHandler = null
      locationStarted = false
      latestLocation = null
      latestGnss = null
      lastRawNmeaAt = 0L
    }
  }

  /** When the chip doesn't deliver raw NMEA, build it from Location + GnssStatus. */
  private fun emitSynthIfNeeded() {
    if (clients.isEmpty()) return
    // Raw NMEA is flowing — don't duplicate it.
    if (lastRawNmeaAt != 0L && SystemClock.elapsedRealtime() - lastRawNmeaAt < RAW_NMEA_GRACE_MS) return
    val loc = latestLocation ?: return
    NmeaSynth.build(loc, latestGnss).forEach { broadcast(it) }
  }
}

/** Builds standard NMEA 0183 sentences from an Android [Location] (+ optional [GnssStatus]). */
private object NmeaSynth {
  fun build(loc: Location, gnss: GnssStatus?): List<String> {
    val out = ArrayList<String>(8)
    val cal = java.util.Calendar.getInstance(java.util.TimeZone.getTimeZone("UTC"))
    cal.timeInMillis = if (loc.time > 0) loc.time else System.currentTimeMillis()
    val hh = cal.get(java.util.Calendar.HOUR_OF_DAY)
    val mm = cal.get(java.util.Calendar.MINUTE)
    val ss = cal.get(java.util.Calendar.SECOND)
    val time = String.format(Locale.US, "%02d%02d%02d.00", hh, mm, ss)
    val date = String.format(
      Locale.US, "%02d%02d%02d",
      cal.get(java.util.Calendar.DAY_OF_MONTH),
      cal.get(java.util.Calendar.MONTH) + 1,
      cal.get(java.util.Calendar.YEAR) % 100
    )

    val lat = coord(abs(loc.latitude), true)
    val latH = if (loc.latitude >= 0) "N" else "S"
    val lon = coord(abs(loc.longitude), false)
    val lonH = if (loc.longitude >= 0) "E" else "W"

    val speedKnots = loc.speed * 1.943844f // m/s -> knots
    val course = loc.bearing
    val alt = if (loc.hasAltitude()) loc.altitude else 0.0
    val sats = gnss?.let { used(it) } ?: 0

    // RMC: time, status, lat, lon, SOG (knots), COG, date.
    out += nmea(
      String.format(
        Locale.US, "GPRMC,%s,A,%s,%s,%s,%s,%.1f,%.1f,%s,,,A",
        time, lat, latH, lon, lonH, speedKnots, course, date
      )
    )
    // GGA: time, lat, lon, fix quality(1), num sats, HDOP, altitude(M), geoid sep.
    out += nmea(
      String.format(
        Locale.US, "GPGGA,%s,%s,%s,%s,%s,1,%02d,%s,%.1f,M,0.0,M,,",
        time, lat, latH, lon, lonH, sats, hdop(loc), alt
      )
    )
    // GSA: auto mode, 3D fix, used PRNs, PDOP/HDOP/VDOP (left blank — gpsd derives them).
    gnss?.let { out += buildGsa(it) }
    // GSV: satellites in view, grouped per constellation talker.
    gnss?.let { out.addAll(buildGsv(it)) }
    return out
  }

  private fun coord(value: Double, isLat: Boolean): String {
    val deg = value.toInt()
    val min = (value - deg) * 60.0
    return if (isLat) String.format(Locale.US, "%02d%07.4f", deg, min)
    else String.format(Locale.US, "%03d%07.4f", deg, min)
  }

  private fun hdop(loc: Location): String {
    // Rough HDOP proxy from horizontal accuracy; gpsd also has GGA's value.
    return if (loc.hasAccuracy()) String.format(Locale.US, "%.1f", (loc.accuracy / 5f).coerceIn(0.5f, 99.9f))
    else "1.0"
  }

  private fun used(gnss: GnssStatus): Int {
    var c = 0
    for (i in 0 until gnss.satelliteCount) if (gnss.usedInFix(i)) c++
    return c
  }

  private fun buildGsa(gnss: GnssStatus): String {
    val prns = ArrayList<Int>()
    for (i in 0 until gnss.satelliteCount) if (gnss.usedInFix(i)) prns.add(gnss.getSvid(i))
    val fields = StringBuilder("GPGSA,A,3")
    for (k in 0 until 12) fields.append(",").append(if (k < prns.size) prns[k].toString() else "")
    fields.append(",,,") // PDOP,HDOP,VDOP left empty
    return nmea(fields.toString())
  }

  private fun buildGsv(gnss: GnssStatus): List<String> {
    // Group satellites by constellation -> talker id, 4 sats per GSV sentence.
    val byTalker = HashMap<String, MutableList<Int>>()
    for (i in 0 until gnss.satelliteCount) {
      val talker = talkerFor(gnss.getConstellationType(i))
      byTalker.getOrPut(talker) { ArrayList() }.add(i)
    }
    val out = ArrayList<String>()
    for ((talker, idx) in byTalker) {
      val total = idx.size
      val sentences = (total + 3) / 4
      for (s in 0 until sentences) {
        val sb = StringBuilder()
        sb.append(talker).append("GSV,").append(sentences).append(",").append(s + 1).append(",")
          .append(String.format(Locale.US, "%02d", total))
        for (k in 0 until 4) {
          val n = s * 4 + k
          if (n < total) {
            val i = idx[n]
            sb.append(",").append(gnss.getSvid(i))
              .append(",").append(gnss.getElevationDegrees(i).toInt())
              .append(",").append(((gnss.getAzimuthDegrees(i).toInt() % 360 + 360) % 360))
              .append(",").append(gnss.getCn0DbHz(i).toInt())
          } else {
            sb.append(",,,,")
          }
        }
        out += nmea(sb.toString())
      }
    }
    return out
  }

  private fun talkerFor(constellation: Int): String = when (constellation) {
    GnssStatus.CONSTELLATION_GPS -> "GP"
    GnssStatus.CONSTELLATION_GLONASS -> "GL"
    GnssStatus.CONSTELLATION_GALILEO -> "GA"
    GnssStatus.CONSTELLATION_BEIDOU -> "GB"
    GnssStatus.CONSTELLATION_QZSS -> "GQ"
    GnssStatus.CONSTELLATION_IRNSS -> "GI"
    GnssStatus.CONSTELLATION_SBAS -> "GP"
    else -> "GP"
  }

  /** Wrap a sentence body in `$...*CS` with the NMEA XOR checksum. */
  private fun nmea(body: String): String {
    var c = 0
    for (ch in body) c = c xor ch.code
    return String.format(Locale.US, "\$%s*%02X", body, c)
  }
}

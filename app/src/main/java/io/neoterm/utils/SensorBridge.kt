package io.neoterm.utils

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.hardware.Sensor
import android.hardware.SensorEvent
import android.hardware.SensorEventListener
import android.hardware.SensorManager
import android.os.BatteryManager
import android.os.Handler
import android.os.HandlerThread
import android.os.SystemClock
import io.neoterm.component.config.NeoPreference
import io.neoterm.component.config.NeoTermPath
import io.neoterm.setup.proot.Kmsg
import java.io.File
import java.util.Locale
import java.util.concurrent.ConcurrentHashMap
import kotlin.math.roundToLong

/**
 * Android-side **sensor + battery bridge** that presents a faithful, kernel-style
 * interface to the proot distro, so the *standard* Linux tools work unmodified —
 * no custom client needed:
 *
 *  - **Battery / power** → a fake `/sys/class/power_supply/{BAT0,AC0,USB}` tree
 *    (capacity/status/voltage_now/temp/current_now/uevent…). `upower`, `acpi`,
 *    `tlp`-style scripts and desktop battery indicators read it directly.
 *  - **Sensors** → a fake `/sys/bus/iio/devices/iio:deviceN/` tree per present
 *    sensor (accel/gyro/magn as `_raw` + `_scale`, light/proximity/pressure/temp/
 *    humidity as `_input`). `iio_info`, `cat`, and any IIO-aware script work; with
 *    `iio-sensor-proxy` installed in the distro, `monitor-sensor` works too.
 *
 * Both trees are real host directories **bound** onto the guest paths (see
 * [sysfsBinds] + ProotManager) — Android SELinux blocks readdir of the real
 * `/sys`, exactly like the USB-serial `/sys/class/tty` bind. proot reads the host
 * dir at access time, so values written here after launch still show up live.
 *
 * Updates are event-driven (a [SensorEventListener] + a sticky battery
 * broadcast), throttled per source and de-duplicated, so an idle sensor costs no
 * I/O. Gated by [NeoPreference.isSensorsEnabled]; started/stopped with the app by
 * NeoTermService. Battery + the eight no-permission sensors only — heart-rate
 * (BODY_SENSORS) and step (ACTIVITY_RECOGNITION) are intentionally excluded.
 */
object SensorBridge {
  private const val TAG = "Sensor"

  /** Minimum gap between two file rewrites for the same source (caps I/O at ~5 Hz). */
  private const val MIN_INTERVAL_MS = 200L

  private val psDir = File("${NeoTermPath.PROOT_ROOT_PATH}/sys-power-supply")
  private val iioDir = File("${NeoTermPath.PROOT_ROOT_PATH}/sys-bus-iio")

  private val batDir = File(psDir, "BAT0")
  private val acDir = File(psDir, "AC0")
  private val usbDir = File(psDir, "USB")

  // No-permission sensors we expose, in iio:deviceN assignment order.
  private val SUPPORTED = intArrayOf(
    Sensor.TYPE_ACCELEROMETER, Sensor.TYPE_GYROSCOPE, Sensor.TYPE_MAGNETIC_FIELD,
    Sensor.TYPE_LIGHT, Sensor.TYPE_PROXIMITY, Sensor.TYPE_PRESSURE,
    Sensor.TYPE_AMBIENT_TEMPERATURE, Sensor.TYPE_RELATIVE_HUMIDITY
  )

  @Volatile private var started = false
  private var appContext: Context? = null
  private var sensorManager: SensorManager? = null
  private var sensorThread: HandlerThread? = null
  private var batteryReceiver: BroadcastReceiver? = null

  private val devDirs = HashMap<Int, File>()       // sensor type -> iio:deviceN dir
  private val lastTick = HashMap<Int, Long>()       // sensor type -> last write (throttle)
  // file path -> last content (de-dup). Concurrent: written from the sensor
  // HandlerThread and the battery-receiver (main) thread, on disjoint key sets.
  private val lastWritten = ConcurrentHashMap<String, String>()

  private val listener = object : SensorEventListener {
    override fun onSensorChanged(e: SensorEvent) {
      val type = e.sensor.type
      val dir = synchronized(devDirs) { devDirs[type] } ?: return
      val now = SystemClock.elapsedRealtime()
      if (now - (lastTick[type] ?: 0L) < MIN_INTERVAL_MS) return
      lastTick[type] = now
      for ((n, c) in dynamicFiles(type, e.values)) write(dir, n, c)
    }

    override fun onAccuracyChanged(s: Sensor?, accuracy: Int) {}
  }

  /** Host dir → guest path binds for the fake /sys trees (empty when disabled).
   *  Called by ProotManager at launch; only the top dirs need to exist at bind
   *  time — the per-device contents are filled in live by [start]. */
  fun sysfsBinds(): List<Pair<String, String>> {
    if (!NeoPreference.isSensorsEnabled()) return emptyList()
    val out = ArrayList<Pair<String, String>>()
    if (psDir.isDirectory || psDir.mkdirs()) out.add(psDir.absolutePath to "/sys/class/power_supply")
    if (iioDir.isDirectory || iioDir.mkdirs()) out.add(iioDir.absolutePath to "/sys/bus/iio/devices")
    return out
  }

  @Synchronized
  fun start(context: Context) {
    if (started) return
    if (!NeoPreference.isSensorsEnabled()) return
    appContext = context.applicationContext
    started = true
    psDir.mkdirs(); iioDir.mkdirs()
    registerBattery()
    registerSensors()
    Kmsg.log("sensors: bridge active — /sys/class/power_supply, /sys/bus/iio/devices")
  }

  @Synchronized
  fun stop() {
    if (!started) return
    started = false
    sensorManager?.let { runCatching { it.unregisterListener(listener) } }
    batteryReceiver?.let { r -> runCatching { appContext?.unregisterReceiver(r) } }
    batteryReceiver = null
    runCatching { sensorThread?.quitSafely() }
    sensorThread = null
    sensorManager = null
    synchronized(devDirs) { devDirs.clear() }
    lastTick.clear()
    lastWritten.clear()
    Kmsg.log("sensors: bridge stopped")
  }

  fun restart(context: Context) { stop(); start(context) }

  // ── sensors (IIO) ────────────────────────────────────────────────────────
  private fun registerSensors() {
    val ctx = appContext ?: return
    val sm = ctx.getSystemService(Context.SENSOR_SERVICE) as? SensorManager ?: return
    sensorManager = sm
    val thread = HandlerThread("sensor-capture").apply { start() }
    sensorThread = thread
    val handler = Handler(thread.looper)
    var idx = 0
    for (type in SUPPORTED) {
      val s = sm.getDefaultSensor(type) ?: continue
      val dir = File(iioDir, "iio:device$idx").apply { mkdirs() }
      synchronized(devDirs) { devDirs[type] = dir }
      write(dir, "name", staticName(type) + "\n")
      staticScale(type)?.let { (n, c) -> write(dir, n, c + "\n") }
      runCatching { sm.registerListener(listener, s, SensorManager.SENSOR_DELAY_NORMAL, handler) }
      Kmsg.log("sensors: iio:device$idx <- ${staticName(type)} (${s.name})")
      idx++
    }
  }

  private fun staticName(type: Int): String = when (type) {
    Sensor.TYPE_ACCELEROMETER -> "accel_3d"
    Sensor.TYPE_GYROSCOPE -> "gyro_3d"
    Sensor.TYPE_MAGNETIC_FIELD -> "magn_3d"
    Sensor.TYPE_LIGHT -> "als"
    Sensor.TYPE_PROXIMITY -> "proximity"
    Sensor.TYPE_PRESSURE -> "pressure"
    Sensor.TYPE_AMBIENT_TEMPERATURE -> "ambient_temp"
    Sensor.TYPE_RELATIVE_HUMIDITY -> "humidity"
    else -> "unknown"
  }

  /** Vector sensors expose raw counts + a fixed scale (processed = raw·scale, SI). */
  private fun staticScale(type: Int): Pair<String, String>? = when (type) {
    Sensor.TYPE_ACCELEROMETER -> "in_accel_scale" to "0.000001"
    Sensor.TYPE_GYROSCOPE -> "in_anglvel_scale" to "0.000001"
    Sensor.TYPE_MAGNETIC_FIELD -> "in_magn_scale" to "0.000001"
    else -> null
  }

  private fun dynamicFiles(type: Int, v: FloatArray): Map<String, String> = when (type) {
    Sensor.TYPE_ACCELEROMETER -> mapOf(
      "in_accel_x_raw" to rawMicro(v, 0), "in_accel_y_raw" to rawMicro(v, 1), "in_accel_z_raw" to rawMicro(v, 2))
    Sensor.TYPE_GYROSCOPE -> mapOf(
      "in_anglvel_x_raw" to rawMicro(v, 0), "in_anglvel_y_raw" to rawMicro(v, 1), "in_anglvel_z_raw" to rawMicro(v, 2))
    Sensor.TYPE_MAGNETIC_FIELD -> mapOf(
      "in_magn_x_raw" to rawMicro(v, 0), "in_magn_y_raw" to rawMicro(v, 1), "in_magn_z_raw" to rawMicro(v, 2))
    Sensor.TYPE_LIGHT -> mapOf("in_illuminance_input" to f3(v, 0))
    Sensor.TYPE_PROXIMITY -> mapOf("in_proximity_input" to f3(v, 0))
    Sensor.TYPE_PRESSURE -> mapOf("in_pressure_input" to f3(v, 0))
    Sensor.TYPE_AMBIENT_TEMPERATURE -> mapOf("in_temp_input" to f3(v, 0))
    Sensor.TYPE_RELATIVE_HUMIDITY -> mapOf("in_humidityrelative_input" to f3(v, 0))
    else -> emptyMap()
  }

  private fun rawMicro(v: FloatArray, i: Int): String =
    (if (i < v.size) (v[i].toDouble() * 1e6).roundToLong() else 0L).toString() + "\n"

  private fun f3(v: FloatArray, i: Int): String =
    String.format(Locale.ROOT, "%.3f", if (i < v.size) v[i] else 0f) + "\n"

  // ── battery (power_supply) ───────────────────────────────────────────────
  private fun registerBattery() {
    batDir.mkdirs(); acDir.mkdirs(); usbDir.mkdirs()
    val r = object : BroadcastReceiver() {
      override fun onReceive(c: Context, i: Intent) = updateBattery(i)
    }
    batteryReceiver = r
    // registerReceiver returns the current sticky ACTION_BATTERY_CHANGED, so we
    // populate immediately and then update on every change.
    val sticky = runCatching {
      appContext?.registerReceiver(r, IntentFilter(Intent.ACTION_BATTERY_CHANGED))
    }.getOrNull()
    sticky?.let { updateBattery(it) }
  }

  private fun updateBattery(i: Intent) {
    val level = i.getIntExtra(BatteryManager.EXTRA_LEVEL, -1)
    val scale = i.getIntExtra(BatteryManager.EXTRA_SCALE, -1)
    val pct = if (level >= 0 && scale > 0) level * 100 / scale else -1
    val status = i.getIntExtra(BatteryManager.EXTRA_STATUS, BatteryManager.BATTERY_STATUS_UNKNOWN)
    val health = i.getIntExtra(BatteryManager.EXTRA_HEALTH, BatteryManager.BATTERY_HEALTH_UNKNOWN)
    val plugged = i.getIntExtra(BatteryManager.EXTRA_PLUGGED, 0)
    val voltageMv = i.getIntExtra(BatteryManager.EXTRA_VOLTAGE, -1)
    val tempTenths = i.getIntExtra(BatteryManager.EXTRA_TEMPERATURE, Int.MIN_VALUE)
    val tech = i.getStringExtra(BatteryManager.EXTRA_TECHNOLOGY)?.takeIf { it.isNotBlank() } ?: "Li-ion"
    val present = i.getBooleanExtra(BatteryManager.EXTRA_PRESENT, true)
    val statusStr = statusStr(status)
    val healthStr = healthStr(health)

    val bm = appContext?.getSystemService(Context.BATTERY_SERVICE) as? BatteryManager
    val curNow = bm?.getIntProperty(BatteryManager.BATTERY_PROPERTY_CURRENT_NOW)
      ?.takeIf { it != Int.MIN_VALUE && it != 0 }
    val chargeCounter = bm?.getIntProperty(BatteryManager.BATTERY_PROPERTY_CHARGE_COUNTER)
      ?.takeIf { it != Int.MIN_VALUE && it != 0 }

    write(batDir, "type", "Battery\n")
    write(batDir, "present", if (present) "1\n" else "0\n")
    write(batDir, "technology", "$tech\n")
    write(batDir, "status", "$statusStr\n")
    if (pct >= 0) {
      write(batDir, "capacity", "$pct\n")
      write(batDir, "capacity_level", "${capLevel(pct, status)}\n")
    }
    write(batDir, "health", "$healthStr\n")
    if (voltageMv >= 0) write(batDir, "voltage_now", "${voltageMv * 1000L}\n")   // µV
    if (tempTenths != Int.MIN_VALUE) write(batDir, "temp", "$tempTenths\n")       // tenths °C
    curNow?.let { write(batDir, "current_now", "$it\n") }                          // µA
    chargeCounter?.let { write(batDir, "charge_counter", "$it\n") }               // µAh
    write(batDir, "uevent", buildUevent(statusStr, present, tech, pct, capLevel(pct, status),
      healthStr, voltageMv, tempTenths, curNow))

    val acOnline = plugged and (BatteryManager.BATTERY_PLUGGED_AC or BatteryManager.BATTERY_PLUGGED_WIRELESS) != 0
    val usbOnline = plugged and BatteryManager.BATTERY_PLUGGED_USB != 0
    write(acDir, "type", "Mains\n"); write(acDir, "online", if (acOnline) "1\n" else "0\n")
    write(usbDir, "type", "USB\n"); write(usbDir, "online", if (usbOnline) "1\n" else "0\n")
  }

  private fun buildUevent(
    status: String, present: Boolean, tech: String, pct: Int, capLevel: String,
    health: String, voltageMv: Int, tempTenths: Int, curNow: Int?
  ): String = buildString {
    append("POWER_SUPPLY_NAME=BAT0\n")
    append("POWER_SUPPLY_TYPE=Battery\n")
    append("POWER_SUPPLY_STATUS=$status\n")
    append("POWER_SUPPLY_PRESENT=${if (present) 1 else 0}\n")
    append("POWER_SUPPLY_TECHNOLOGY=$tech\n")
    if (pct >= 0) { append("POWER_SUPPLY_CAPACITY=$pct\n"); append("POWER_SUPPLY_CAPACITY_LEVEL=$capLevel\n") }
    append("POWER_SUPPLY_HEALTH=$health\n")
    if (voltageMv >= 0) append("POWER_SUPPLY_VOLTAGE_NOW=${voltageMv * 1000L}\n")
    if (tempTenths != Int.MIN_VALUE) append("POWER_SUPPLY_TEMP=$tempTenths\n")
    curNow?.let { append("POWER_SUPPLY_CURRENT_NOW=$it\n") }
  }

  private fun statusStr(s: Int): String = when (s) {
    BatteryManager.BATTERY_STATUS_CHARGING -> "Charging"
    BatteryManager.BATTERY_STATUS_DISCHARGING -> "Discharging"
    BatteryManager.BATTERY_STATUS_FULL -> "Full"
    BatteryManager.BATTERY_STATUS_NOT_CHARGING -> "Not charging"
    else -> "Unknown"
  }

  private fun healthStr(h: Int): String = when (h) {
    BatteryManager.BATTERY_HEALTH_GOOD -> "Good"
    BatteryManager.BATTERY_HEALTH_OVERHEAT -> "Overheat"
    BatteryManager.BATTERY_HEALTH_DEAD -> "Dead"
    BatteryManager.BATTERY_HEALTH_OVER_VOLTAGE -> "Over voltage"
    BatteryManager.BATTERY_HEALTH_COLD -> "Cold"
    BatteryManager.BATTERY_HEALTH_UNSPECIFIED_FAILURE -> "Unspecified failure"
    else -> "Unknown"
  }

  private fun capLevel(pct: Int, status: Int): String = when {
    status == BatteryManager.BATTERY_STATUS_FULL || pct >= 100 -> "Full"
    pct in 0..5 -> "Critical"
    pct in 6..15 -> "Low"
    pct >= 80 -> "High"
    else -> "Normal"
  }

  // ── file writer (de-duplicated) ──────────────────────────────────────────
  private fun write(dir: File, name: String, content: String) {
    val f = File(dir, name)
    val path = f.absolutePath
    if (lastWritten[path] == content) return
    runCatching { f.writeText(content) }.onSuccess { lastWritten[path] = content }
  }
}

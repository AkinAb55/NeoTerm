package io.neoterm.frontend.session.terminal

import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.PendingIntent
import android.content.ClipData
import android.content.ClipboardManager
import android.content.Context
import android.content.Intent
import android.media.AudioAttributes
import android.media.AudioManager
import android.media.RingtoneManager
import android.os.SystemClock
import android.os.VibrationEffect
import android.os.Vibrator
import android.util.Base64
import android.util.Log
import android.view.KeyEvent
import android.view.MotionEvent
import android.view.View
import android.view.inputmethod.InputMethodManager
import androidx.core.app.NotificationCompat
import io.neoterm.App
import io.neoterm.BuildConfig
import io.neoterm.R
import io.neoterm.backend.KeyHandler
import io.neoterm.backend.TerminalSession
import io.neoterm.component.ComponentManager
import io.neoterm.component.completion.*
import io.neoterm.component.config.NeoPreference
import io.neoterm.component.extrakey.ExtraKeyComponent
import io.neoterm.component.session.ShellTermSession
import io.neoterm.frontend.completion.CandidatePopupWindow
import io.neoterm.frontend.session.view.TerminalView
import io.neoterm.frontend.session.view.TerminalViewClient
import java.util.*

/**
 * @author kiva
 */
class TermViewClient(val context: Context) : TerminalViewClient {
  private var mVirtualControlKeyDown: Boolean = false
  private var mVirtualFnKeyDown: Boolean = false
  private var lastTitle: String = ""

  var termSessionData: TermSessionData? = null

  override fun onScale(scale: Float): Float {
    if (scale < 0.9f || scale > 1.1f) {
      val increase = scale > 1f
      changeFontSize(increase)
      return 1.0f
    }
    return scale
  }

  override fun onSingleTapUp(e: MotionEvent?) {
    val termView = termSessionData?.termView ?: return
    (context.getSystemService(Context.INPUT_METHOD_SERVICE) as InputMethodManager)
      .showSoftInput(termView, InputMethodManager.SHOW_IMPLICIT)
  }

  override fun shouldBackButtonBeMappedToEscape(): Boolean {
    val shellSession = termSessionData?.termSession as ShellTermSession? ?: return false
    return shellSession.shellProfile.enableBackKeyToEscape
  }

  override fun copyModeChanged(copyMode: Boolean) {
    // Keep the soft keyboard visible while selecting text (the user asked for
    // it to stay up in selection mode), so do nothing here.
  }

  override fun onKeyDown(keyCode: Int, e: KeyEvent?, session: TerminalSession?): Boolean {
    if (handleVirtualKeys(keyCode, e, true)) {
      return true
    }

    val termUI = termSessionData?.termUI

    when (keyCode) {
      KeyEvent.KEYCODE_ENTER -> {
        if (e?.action == KeyEvent.ACTION_DOWN && session?.isRunning == false) {
          termUI?.requireClose()
          return true
        }
        return false
      }
      KeyEvent.KEYCODE_BACK -> {
        if (e?.action == KeyEvent.ACTION_DOWN) {
          return termUI?.requireFinishAutoCompletion() ?: false
        }
        return false
      }
    }

    // TODO 自定义快捷键
    if (e != null && e.isCtrlPressed && e.isShiftPressed) {
      // Get the unmodified code point:
      val unicodeChar = e.getUnicodeChar(0).toChar()

      when (unicodeChar) {
        'v' -> termUI?.requirePaste()
        'n' -> termUI?.requireCreateNew()
        'w' -> termUI?.requireClose()
        'z' -> termUI?.requireSwitchToPrevious()
        'x' -> termUI?.requireSwitchToNext()
        'f' -> termUI?.requireToggleFullScreen()
        '-' -> changeFontSize(false)
        '+' -> changeFontSize(true)
      }

      // 当要触发 NeoTerm 快捷键时，屏蔽所有终端处理key
      return true
    } else if (e != null && e.isAltPressed) {
      // Get the unmodified code point:
      val unicodeChar = e.getUnicodeChar(0).toChar()
      if (unicodeChar !in ('1'..'9')) {
        return false
      }

      // Use Alt + num to switch sessions
      val sessionIndex = unicodeChar.toInt() - '0'.toInt()
      termUI?.requireSwitchTo(sessionIndex)

      // 当要触发 NeoTerm 快捷键时，屏蔽所有终端处理key
      return true
    }
    return false
  }

  override fun onKeyUp(keyCode: Int, e: KeyEvent?): Boolean {
    return handleVirtualKeys(keyCode, e, false)
  }

  override fun readControlKey(): Boolean {
    val extraKeysView = termSessionData?.extraKeysView
    return (extraKeysView != null && extraKeysView.readControlButton()) || mVirtualControlKeyDown
  }

  override fun readAltKey(): Boolean {
    val extraKeysView = termSessionData?.extraKeysView
    return (extraKeysView != null && extraKeysView.readAltButton()) || mVirtualFnKeyDown
  }

  override fun onCodePoint(codePoint: Int, ctrlDown: Boolean, session: TerminalSession?): Boolean {
    if (mVirtualFnKeyDown) {
      var resultingKeyCode: Int = -1
      var resultingCodePoint: Int = -1
      var altDown = false
      val lowerCase = Character.toLowerCase(codePoint)
      when (lowerCase.toChar()) {
        // Arrow keys.
        'w' -> resultingKeyCode = KeyEvent.KEYCODE_DPAD_UP
        'a' -> resultingKeyCode = KeyEvent.KEYCODE_DPAD_LEFT
        's' -> resultingKeyCode = KeyEvent.KEYCODE_DPAD_DOWN
        'd' -> resultingKeyCode = KeyEvent.KEYCODE_DPAD_RIGHT

        // Page up and down.
        'p' -> resultingKeyCode = KeyEvent.KEYCODE_PAGE_UP
        'n' -> resultingKeyCode = KeyEvent.KEYCODE_PAGE_DOWN

        // Some special keys:
        't' -> resultingKeyCode = KeyEvent.KEYCODE_TAB
        'i' -> resultingKeyCode = KeyEvent.KEYCODE_INSERT
        'h' -> resultingCodePoint = '~'.toInt()

        // Special characters to input.
        'u' -> resultingCodePoint = '_'.toInt()
        'l' -> resultingCodePoint = '|'.toInt()

        // Function keys.
        '1', '2', '3', '4', '5', '6', '7', '8', '9' -> resultingKeyCode = codePoint - '1'.toInt() + KeyEvent.KEYCODE_F1
        '0' -> resultingKeyCode = KeyEvent.KEYCODE_F10

        // Other special keys.
        'e' -> resultingCodePoint = 27 /*Escape*/
        '.' -> resultingCodePoint = 28 /*^.*/

        'b' // alt+b, jumping backward in readline.
          , 'f' // alf+f, jumping forward in readline.
          , 'x' // alt+x, common in emacs.
        -> {
          resultingCodePoint = lowerCase
          altDown = true
        }

        // Volume control.
        'v' -> {
          resultingCodePoint = -1
          val audio = context.getSystemService(Context.AUDIO_SERVICE) as AudioManager
          audio.adjustSuggestedStreamVolume(
            AudioManager.ADJUST_SAME,
            AudioManager.USE_DEFAULT_STREAM_TYPE,
            AudioManager.FLAG_SHOW_UI
          )
        }
      }

      if (resultingKeyCode != -1) {
        if (session != null) {
          val term = session.emulator
          session.write(
            KeyHandler.getCode(
              resultingKeyCode,
              0,
              term.isCursorKeysApplicationMode,
              term.isKeypadApplicationMode
            )
          )
        }
      } else if (resultingCodePoint != -1) {
        session?.writeCodePoint(altDown, resultingCodePoint)
      }
      return true
    }
    return false
  }

  override fun onLongPress(event: MotionEvent?): Boolean {
    // Text selection is started by the TerminalView itself on long press; the
    // client needs no extra handling here.
    return false
  }

  override fun onCustomCommands(session: TerminalSession?) {
    // The "CC" button: open the custom-commands manager. A chosen command runs
    // either in the active session (write to it) or in a freshly opened one.
    io.neoterm.ui.term.CustomCommandsDialog(
      context,
      runInCurrent = { cmd -> session?.write(cmd + "\n") },
      runInNew = { cmd ->
        org.greenrobot.eventbus.EventBus.getDefault().post(CreateNewSessionEvent(cmd))
      }
    ).show()
  }

  override fun onSwipe(toLeft: Boolean) {
    // Page between tabs: swipe left -> next session, swipe right -> previous.
    val termUI = termSessionData?.termUI ?: return
    if (toLeft) termUI.requireSwitchToNext() else termUI.requireSwitchToPrevious()
  }

  private fun handleVirtualKeys(keyCode: Int, event: KeyEvent?, down: Boolean): Boolean {
    if (event == null) {
      return false
    }
    // Only the volume keys are candidates for the special-key mapping.
    if (keyCode != KeyEvent.KEYCODE_VOLUME_DOWN && keyCode != KeyEvent.KEYCODE_VOLUME_UP) {
      return false
    }

    val shellSession = termSessionData?.termSession as ShellTermSession? ?: return false

    // Honor the "use volume keys as special keys" setting directly: when it's
    // off we DON'T consume the event, so the volume keys keep controlling the
    // system volume. (Previously the keys were swallowed even when disabled.)
    //
    // We intentionally do NOT gate on event.device.keyboardType here: on many
    // phones (e.g. Samsung) the built-in device that emits the volume keycodes
    // reports KEYBOARD_TYPE_ALPHABETIC, which made the old guard bail out and
    // the toggle appear to do nothing.
    if (!shellSession.shellProfile.enableSpecialVolumeKeys) {
      return false
    }

    if (keyCode == KeyEvent.KEYCODE_VOLUME_DOWN) {
      mVirtualControlKeyDown = down   // Volume Down → Ctrl
    } else {
      mVirtualFnKeyDown = down         // Volume Up → Fn
    }
    return true
  }

  fun updateExtraKeys(title: String?, force: Boolean = false) {
    val extraKeysView = termSessionData?.extraKeysView ?: return

    // Always apply the show/hide setting first, even before the shell has set a
    // title (e.g. a bare system shell), so the extra-keys visibility matches the
    // preference for every session, not just ones that report a title.
    val enabled = updateExtraKeysVisibility()
    if (!enabled || title == null || title.isEmpty()) {
      return
    }

    if (lastTitle != title || force) {
      removeExtraKeys()
      ComponentManager.getComponent<ExtraKeyComponent>().showShortcutKeys(title, extraKeysView)
      extraKeysView.updateButtons()
      lastTitle = title
    }
  }

  private fun updateExtraKeysVisibility(): Boolean {
    val extraKeysView = termSessionData?.extraKeysView ?: return false
    val shellSession = termSessionData?.termSession as ShellTermSession? ?: return false

    return if (shellSession.shellProfile.enableExtraKeys) {
      extraKeysView.visibility = View.VISIBLE
      true
    } else {
      extraKeysView.visibility = View.GONE
      false
    }
  }

  private fun removeExtraKeys() {
    val extraKeysView = termSessionData?.extraKeysView
    extraKeysView?.clearUserKeys()
  }

  private fun changeFontSize(increase: Boolean) {
    val termView = termSessionData?.termView ?: return
    val changedSize = (if (increase) 1 else -1) * 2
    val fontSize = NeoPreference.validateFontSize(termView.textSize + changedSize)
    NeoPreference.store(NeoPreference.KEY_FONT_SIZE, fontSize)
    // Font size is a global preference: broadcast it so every open tab updates uniformly,
    // not just the one that was pinch-zoomed / received the +/- key (handled by the activity).
    org.greenrobot.eventbus.EventBus.getDefault().post(FontSizeChangedEvent(fontSize))
  }
}

/**
 * @author kiva
 */
class TermSessionCallback : TerminalSession.SessionChangedCallback {
  var termSessionData: TermSessionData? = null

  var bellController: BellController? = null

  var oscNotificationController: OscNotificationController? = null

  override fun onTextChanged(changedSession: TerminalSession?) {
    termSessionData?.termView?.onScreenUpdated()
  }

  override fun onTitleChanged(changedSession: TerminalSession?) {
    if (changedSession?.title != null) {
      termSessionData?.termUI?.requireUpdateTitle(changedSession.title)
    }
  }

  override fun onSessionFinished(finishedSession: TerminalSession?) {
    // Exit status is set just before this callback fires.
    val exitCode = finishedSession?.exitStatus ?: 0
    termSessionData?.termUI?.requireOnSessionFinished(exitCode)
  }

  override fun onClipboardText(session: TerminalSession?, text: String?) {
    val termView = termSessionData?.termView
    if (termView != null) {
      val clipboard = termView.context.getSystemService(Context.CLIPBOARD_SERVICE) as ClipboardManager
      clipboard.setPrimaryClip(ClipData.newPlainText("", text))
    }
  }

  override fun onBell(session: TerminalSession?) {
    val termView = termSessionData?.termView ?: return
    val shellSession = session as ShellTermSession

    if (bellController == null) {
      bellController = BellController()
    }

    bellController?.bellOrVibrate(termView.context, shellSession)
  }

  override fun onColorsChanged(session: TerminalSession?) {
    val termView = termSessionData?.termView
    if (session != null && termView != null) {
      termView.onScreenUpdated()
      // Keep a co-operating on-screen keyboard in sync with the live terminal
      // colors (e.g. when a program changes the background via an OSC escape).
      termView.onTerminalColorsChanged()
    }
  }

  override fun onNotification(session: TerminalSession?, oscCode: Int, params: String?) {
    if (params == null) return
    if (oscNotificationController == null) {
      oscNotificationController = OscNotificationController()
    }
    // Use the application context so notifications still post when the terminal
    // view isn't attached (the whole point is to alert in the background).
    oscNotificationController?.handle(App.get(), oscCode, params)
  }
}

class BellController {
  companion object {
    private const val BELL_DELAY_MS = 100L
    private const val BELL_CHANNEL_ID = "neoterm_bell"
    private const val BELL_NOTIFICATION_ID = 52020
  }

  private var lastBellTime = 0L
  private var channelReady = false

  fun bellOrVibrate(context: Context, session: ShellTermSession) {
    val currentTime = System.currentTimeMillis()
    if (currentTime - lastBellTime < BELL_DELAY_MS) {
      return
    }
    lastBellTime = currentTime

    val app = context.applicationContext

    if (session.shellProfile.enableBell) {
      // The terminal bell is delivered as a notification using the system's
      // default notification sound, so it sounds consistent with the rest of
      // the system and also alerts the user when the app is in the background.
      postBellNotification(app)
    }

    if (session.shellProfile.enableVibrate) {
      val vibrator = app.getSystemService(Context.VIBRATOR_SERVICE) as? Vibrator
      if (vibrator != null && vibrator.hasVibrator()) {
        vibrator.vibrate(VibrationEffect.createOneShot(120, VibrationEffect.DEFAULT_AMPLITUDE))
      }
    }
  }

  private fun postBellNotification(context: Context) {
    val manager = context.getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
    ensureChannel(context, manager)

    val launch = context.packageManager.getLaunchIntentForPackage(context.packageName)
      ?.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
    val contentIntent = if (launch != null) {
      PendingIntent.getActivity(
        context, 0, launch,
        PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE
      )
    } else null

    val builder = NotificationCompat.Builder(context, BELL_CHANNEL_ID)
      .setSmallIcon(R.drawable.ic_terminal_running)
      .setContentTitle(context.getString(R.string.app_name))
      .setContentText(context.getString(R.string.bell_notification_text))
      .setAutoCancel(true)
      .setCategory(NotificationCompat.CATEGORY_MESSAGE)
      .setPriority(NotificationCompat.PRIORITY_HIGH)
      .setTimeoutAfter(4000)
    if (contentIntent != null) builder.setContentIntent(contentIntent)

    manager.notify(BELL_NOTIFICATION_ID, builder.build())
  }

  private fun ensureChannel(context: Context, manager: NotificationManager) {
    if (channelReady) return
    val channel = NotificationChannel(
      BELL_CHANNEL_ID,
      context.getString(R.string.bell_channel_name),
      NotificationManager.IMPORTANCE_HIGH
    )
    val sound = RingtoneManager.getDefaultUri(RingtoneManager.TYPE_NOTIFICATION)
    if (sound != null) {
      val attrs = AudioAttributes.Builder()
        .setUsage(AudioAttributes.USAGE_NOTIFICATION_EVENT)
        .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
        .build()
      channel.setSound(sound, attrs)
    }
    // Vibration is governed by the separate "Vibrate" setting, not the channel.
    channel.enableVibration(false)
    channel.setShowBadge(false)
    manager.createNotificationChannel(channel)
    channelReady = true
  }
}

/**
 * Posts Android notifications requested by programs in the terminal via OSC
 * escape sequences. Supports the three de-facto protocols:
 *
 *  - **OSC 9**   `ESC ] 9 ; <body> BEL`                 (iTerm2 / Windows Terminal)
 *  - **OSC 777** `ESC ] 777 ; notify ; <title> ; <body>`(urxvt)
 *  - **OSC 99**  `ESC ] 99 ; <metadata> ; <payload>`    (kitty — fully parameterized:
 *                title/body, id for update/replace, urgency, base64 payloads, chunking)
 *
 * Gated behind the user's settings (master toggle + sound + urgency sub-toggles);
 * off by default because any terminal output (including remote SSH) can trigger it.
 * A light rate-limit guards against floods.
 */
class OscNotificationController {
  companion object {
    private const val CHANNEL_PREFIX = "neoterm_osc"
    private const val BASE_NOTIFICATION_ID = 53000
    private const val MIN_INTERVAL_MS = 150L
    private const val MAX_PENDING = 16

    // kitty urgency levels.
    private const val URGENCY_LOW = 0
    private const val URGENCY_NORMAL = 1
    private const val URGENCY_CRITICAL = 2
  }

  /** Accumulates a kitty (OSC 99) notification across chunks, keyed by its id. */
  private class Pending99 {
    val title = StringBuilder()
    val body = StringBuilder()
    var urgency = URGENCY_NORMAL

    /** True once the sender marked the notification complete (d=1). */
    var sealed = false

    fun reset() {
      title.setLength(0)
      body.setLength(0)
      urgency = URGENCY_NORMAL
      sealed = false
    }
  }

  private val createdChannels = HashSet<String>()
  private val pending = LinkedHashMap<String, Pending99>()
  private var idlessCounter = 0
  private var lastPostTime = 0L
  private var lastNotifId = -1

  fun handle(context: Context, oscCode: Int, params: String) {
    if (!NeoPreference.isOscNotificationEnabled()) return
    val app = context.applicationContext
    when (oscCode) {
      9 -> handleOsc9(app, params)
      777 -> handleOsc777(app, params)
      99 -> handleOsc99(app, params)
    }
  }

  // OSC 9: "<body>". ConEmu overloads OSC 9 with "9;<n>;..." control commands
  // (progress, cwd, …); skip those so we only react to iTerm2-style notifications.
  private fun handleOsc9(context: Context, params: String) {
    if (params.isEmpty()) return
    val sep = params.indexOf(';')
    if (sep >= 0 && params.substring(0, sep).toIntOrNull() != null) return
    post(context, nextIdlessId(), "", params, URGENCY_NORMAL)
  }

  // OSC 777: "notify;<title>;<body>".
  private fun handleOsc777(context: Context, params: String) {
    val parts = params.split(';', limit = 3)
    if (parts.firstOrNull() != "notify") return
    val title = parts.getOrElse(1) { "" }
    val body = parts.getOrElse(2) { "" }
    if (title.isEmpty() && body.isEmpty()) return
    post(context, nextIdlessId(), title, body, URGENCY_NORMAL)
  }

  // OSC 99 (kitty): "<metadata>;<payload>". metadata is a colon-separated list of
  // key=value pairs (e.g. "i=1:d=0:p=body"):
  //   i=<id>  p=title|body|close  e=0|1 (base64)  d=0|1 (done)  u=0|1|2 (urgency)
  //
  // Title and body arrive as separate chunks; we accumulate them per id and
  // (re)post the SAME notification on every chunk so the body shows up and a
  // later chunk with the same id updates in place. A title chunk arriving after
  // the notification was completed (sealed) starts a fresh notification (so the
  // same id can be reused to replace), while a body chunk after completion just
  // adds the body to the existing one.
  private fun handleOsc99(context: Context, params: String) {
    val sep = params.indexOf(';')
    val meta = if (sep >= 0) params.substring(0, sep) else params
    val payloadRaw = if (sep >= 0) params.substring(sep + 1) else ""

    var id = ""
    var part = "title"
    var encoded = false
    var done = true
    var urgency = URGENCY_NORMAL
    for (kv in meta.split(':')) {
      if (kv.isEmpty()) continue
      val eq = kv.indexOf('=')
      val key = if (eq >= 0) kv.substring(0, eq) else kv
      val value = if (eq >= 0) kv.substring(eq + 1) else ""
      when (key) {
        "i" -> id = value
        "p" -> part = value
        "e" -> encoded = value == "1"
        "d" -> done = value != "0"
        "u" -> urgency = value.toIntOrNull() ?: URGENCY_NORMAL
      }
    }

    if (part == "close") {
      cancel(context, id)
      pending.remove(id)
      return
    }

    val payload = if (encoded) decodeBase64(payloadRaw) else payloadRaw

    // Notifications without an id are one-shot (each escape is independent).
    if (id.isEmpty()) {
      val title = if (part == "body") "" else payload
      val body = if (part == "body") payload else ""
      post(context, nextIdlessId(), title, body, urgency)
      return
    }

    val buf = pending.getOrPut(id) {
      // Cap memory: drop the oldest tracked notification if the map grows.
      if (pending.size >= MAX_PENDING) pending.keys.firstOrNull()?.let { pending.remove(it) }
      Pending99()
    }
    if (buf.sealed) {
      // The previous notification for this id was completed. A new title starts
      // a fresh one; a new body extends the existing one.
      if (part != "body") buf.reset() else buf.sealed = false
    }
    buf.urgency = urgency
    if (part == "body") buf.body.append(payload) else buf.title.append(payload)

    post(context, notifIdFor(id), buf.title.toString(), buf.body.toString(), buf.urgency)
    if (done) buf.sealed = true
  }

  private fun decodeBase64(s: String): String = try {
    String(Base64.decode(s, Base64.DEFAULT), Charsets.UTF_8)
  } catch (e: IllegalArgumentException) {
    s // Not valid base64 — fall back to the raw text.
  }

  private fun nextIdlessId(): Int = BASE_NOTIFICATION_ID + 100 + (idlessCounter++ and 0x3f)

  private fun notifIdFor(id: String): Int = BASE_NOTIFICATION_ID + (id.hashCode() and 0x7f)

  private fun cancel(context: Context, id: String) {
    val manager = context.getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
    manager.cancel(notifIdFor(id))
  }

  private fun post(context: Context, notifId: Int, title: String, body: String, urgency: Int) {
    // Always allow updating the SAME notification (e.g. a title chunk followed by
    // a body chunk, or a same-id replace). Only throttle bursts of NEW (distinct)
    // notifications, to guard against floods without dropping legitimate updates.
    val now = SystemClock.elapsedRealtime()
    if (notifId != lastNotifId && now - lastPostTime < MIN_INTERVAL_MS) return
    lastPostTime = now
    lastNotifId = notifId

    val sound = NeoPreference.isOscNotificationSoundEnabled()
    val importance = importanceFor(urgency)
    val channelId = channelFor(context, importance, sound)

    val launch = context.packageManager.getLaunchIntentForPackage(context.packageName)
      ?.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
    val contentIntent = if (launch != null) {
      PendingIntent.getActivity(
        context, 0, launch,
        PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE
      )
    } else null

    val shownTitle = title.ifEmpty { context.getString(R.string.osc_notification_default_title) }
    val builder = NotificationCompat.Builder(context, channelId)
      .setSmallIcon(R.drawable.ic_terminal_running)
      .setContentTitle(shownTitle)
      .setAutoCancel(true)
      .setCategory(NotificationCompat.CATEGORY_MESSAGE)
      .setPriority(priorityFor(urgency))
      // Title and body arrive as separate chunks that re-post the same id; only
      // alert (sound/heads-up) on the first so chunked updates don't double-buzz.
      .setOnlyAlertOnce(true)
    if (body.isNotEmpty()) {
      builder.setContentText(body)
      builder.setStyle(NotificationCompat.BigTextStyle().bigText(body))
    }
    if (!sound) builder.setSilent(true)
    if (contentIntent != null) builder.setContentIntent(contentIntent)

    val manager = context.getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
    manager.notify(notifId, builder.build())
  }

  private fun importanceFor(urgency: Int): Int {
    if (!NeoPreference.isOscNotificationUrgencyEnabled()) {
      return NotificationManager.IMPORTANCE_DEFAULT
    }
    return when (urgency) {
      URGENCY_LOW -> NotificationManager.IMPORTANCE_LOW
      URGENCY_CRITICAL -> NotificationManager.IMPORTANCE_HIGH
      else -> NotificationManager.IMPORTANCE_DEFAULT
    }
  }

  private fun priorityFor(urgency: Int): Int {
    if (!NeoPreference.isOscNotificationUrgencyEnabled()) {
      return NotificationCompat.PRIORITY_DEFAULT
    }
    return when (urgency) {
      URGENCY_LOW -> NotificationCompat.PRIORITY_LOW
      URGENCY_CRITICAL -> NotificationCompat.PRIORITY_HIGH
      else -> NotificationCompat.PRIORITY_DEFAULT
    }
  }

  // A channel's importance and sound are immutable once created, so we use a
  // distinct channel per (importance, sound) combination, created on demand.
  private fun channelFor(context: Context, importance: Int, sound: Boolean): String {
    val id = "${CHANNEL_PREFIX}_${importance}_${if (sound) "snd" else "sil"}"
    if (createdChannels.add(id)) {
      val channel = NotificationChannel(
        id, context.getString(R.string.osc_notification_channel_name), importance
      )
      if (sound) {
        val uri = RingtoneManager.getDefaultUri(RingtoneManager.TYPE_NOTIFICATION)
        if (uri != null) {
          val attrs = AudioAttributes.Builder()
            .setUsage(AudioAttributes.USAGE_NOTIFICATION)
            .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
            .build()
          channel.setSound(uri, attrs)
        }
      } else {
        channel.setSound(null, null)
      }
      val manager = context.getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
      manager.createNotificationChannel(channel)
    }
    return id
  }
}

class TermCompleteListener(var terminalView: TerminalView?) : OnAutoCompleteListener, OnCandidateSelectedListener {
  private val inputStack = Stack<Char>()
  private var popupWindow: CandidatePopupWindow? = null
  private var lastCompletedIndex = 0

  override fun onKeyCode(keyCode: Int, keyMod: Int) {
    when (keyCode) {
      KeyEvent.KEYCODE_DEL -> {
        popChar()
        fixLastCompletedIndex()
        triggerCompletion()
      }

      KeyEvent.KEYCODE_ENTER -> {
        clearChars()
        popupWindow?.dismiss()
      }
    }
  }

  private fun fixLastCompletedIndex() {
    val currentText = getCurrentEditingText()
    lastCompletedIndex = minOf(lastCompletedIndex, currentText.length - 1)
  }

  override fun onCompletionRequired(newText: String?) {
    if (newText == null || newText.isEmpty()) {
      return
    }
    pushString(newText)
    triggerCompletion()
  }

  override fun onCleanUp() {
    popupWindow?.dismiss()
    popupWindow?.cleanup()
    popupWindow = null
    terminalView = null
  }

  override fun onFinishCompletion(): Boolean {
    val popWindow = popupWindow ?: return false

    if (popWindow.isShowing()) {
      popWindow.dismiss()
      return true
    }
    return false
  }

  override fun onCandidateSelected(candidate: CompletionCandidate) {
    val session = terminalView?.currentSession ?: return
    val textNeedCompletion = getCurrentEditingText().substring(lastCompletedIndex + 1)
    val newText = candidate.completeString

    val deleteLength = newText.indexOf(textNeedCompletion) + textNeedCompletion.length
    if (deleteLength > 0) {
      for (i in 0 until deleteLength) {
        session.write("\b")
        popChar()
      }
    }

    if (BuildConfig.DEBUG) {
      Log.e(
        "NeoTerm-AC",
        "currentEditing: $textNeedCompletion, " +
          "deleteLength: $deleteLength, completeString: $newText"
      )
    }

    pushString(newText)
    session.write(newText)
    // Trigger next completion
    lastCompletedIndex = inputStack.size
    triggerCompletion()
  }

  private fun triggerCompletion() {
    val text = getCurrentEditingText()
    if (text.isEmpty()) {
      return
    }

    val result = CompletionManager.tryCompleteFor(text)
    if (!result.hasResult()) {
      // A provider accepted the task
      // But no candidates are provided
      // Give it zero angrily!
      result.markScore(0)
      onFinishCompletion()
      return
    }
    showAutoCompleteCandidates(result)
  }

  private fun showAutoCompleteCandidates(result: CompletionResult) {
    val termView = terminalView
    var popWindow = popupWindow

    if (termView == null) {
      return
    }

    if (popWindow == null) {
      popWindow = CandidatePopupWindow(termView.context)
      popWindow.onCandidateSelectedListener = this
      this.popupWindow = popWindow
    }

    popWindow.candidates = result.candidates
    popWindow.show(termView)
  }

  private fun getCurrentEditingText(): String {
    val builder = StringBuilder()
    val size = inputStack.size
    var start = inputStack.lastIndexOf(' ')
    if (start < 0) {
      // Yes, it is -1, we will do `start + 1` below.
      start = -1
    }

    IntRange(start + 1, size - 1)
      .map { inputStack[it] }
      .takeWhile { !(it == 0.toChar() || it == ' ') }
      .forEach { builder.append(it) }
    return builder.toString()
  }

  private fun clearChars() {
    inputStack.clear()
    lastCompletedIndex = 0
  }

  private fun popChar() {
    if (inputStack.isNotEmpty()) {
      inputStack.pop()
    }
  }

  private fun pushString(string: String) {
    string.toCharArray().forEach { pushChar(it) }
  }

  private fun pushChar(char: Char) {
    inputStack.push(char)
  }
}

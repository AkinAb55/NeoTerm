package com.termux.x11;

import android.annotation.SuppressLint;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.os.IBinder;
import android.os.PowerManager;
import android.system.Os;
import android.util.Log;

import androidx.annotation.Keep;
import androidx.annotation.Nullable;

/**
 * NeoTerm-owned host for the embedded X server. Declared with
 * android:process=":x11server", so it runs in its own process — mirroring
 * Termux:X11's dedicated server process. This:
 *   - keeps NeoTerm's (and the GUI's) main thread free, so the app doesn't
 *     freeze while the server renders;
 *   - is an AMS-managed foreground service, not a forked child, so it survives
 *     Android 12+ phantom-process killing.
 *
 * The X server reads its config from environment variables, which are per-
 * process, so NeoTerm passes TMPDIR (the proot-shared socket dir) and
 * XKB_CONFIG_ROOT (the distro's xkb data) as intent extras and we apply them
 * here before starting the server.
 */
@Keep
public class NeoX11Service extends Service {
    public static final String EXTRA_TMPDIR = "tmpdir";
    public static final String EXTRA_XKB = "xkb_config_root";

    private static final String CHANNEL = "neoterm_x11";
    private static final int NOTI_ID = 0x4011;
    private static boolean started = false;

    @SuppressWarnings("FieldCanBeLocal")
    private PowerManager.WakeLock wakeLock = null;

    @Override
    public void onCreate() {
        super.onCreate();
        startForeground(NOTI_ID, buildNotification());
    }

    @SuppressLint("WakelockTimeout")
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (!started) {
            started = true;
            try {
                // Keep the CPU awake so the X server keeps serving even with the
                // screen off (on by default, matching the terminal).
                PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
                wakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "NeoTerm:x11");
                wakeLock.acquire();
                if (intent != null) {
                    String tmp = intent.getStringExtra(EXTRA_TMPDIR);
                    String xkb = intent.getStringExtra(EXTRA_XKB);
                    if (tmp != null) trySetenv("TMPDIR", tmp);
                    if (xkb != null) trySetenv("XKB_CONFIG_ROOT", xkb);
                }
                System.loadLibrary("Xlorie");
                CmdEntryPoint.startInProcess(getApplicationContext(), new String[]{":0"});
                Log.i("NeoX11Service", "in-process X server requested on :0");
            } catch (Throwable t) {
                Log.e("NeoX11Service", "failed to start X server", t);
            }
        }
        return START_STICKY;
    }

    private void trySetenv(String key, String value) {
        try {
            Os.setenv(key, value, true);
        } catch (Throwable t) {
            Log.e("NeoX11Service", "setenv " + key + " failed", t);
        }
    }

    private Notification buildNotification() {
        NotificationManager nm = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
        Notification.Builder b;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel ch = new NotificationChannel(
                    CHANNEL, "NeoTerm X11", NotificationManager.IMPORTANCE_LOW);
            ch.setShowBadge(false);
            nm.createNotificationChannel(ch);
            b = new Notification.Builder(this, CHANNEL);
        } else {
            //noinspection deprecation
            b = new Notification.Builder(this);
        }
        return b.setContentTitle("NeoTerm X11")
                .setContentText("X server running on :0")
                .setSmallIcon(android.R.drawable.ic_menu_view)
                .setOngoing(true)
                .build();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (wakeLock != null && wakeLock.isHeld()) {
            try { wakeLock.release(); } catch (Throwable ignored) {}
        }
        // This is a dedicated process; kill it so the native X server actually
        // exits (its threads would otherwise keep running until the process dies).
        android.os.Process.killProcess(android.os.Process.myPid());
    }

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
}

package com.termux.x11;

import static android.system.Os.getuid;
import static android.system.Os.getenv;

import android.annotation.SuppressLint;
import android.app.IActivityManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.IIntentReceiver;
import android.content.IIntentSender;
import android.content.Intent;
import android.os.Binder;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.ParcelFileDescriptor;
import android.os.RemoteException;
import android.util.Log;
import android.view.Surface;

import androidx.annotation.Keep;

import java.io.OutputStream;
import java.io.PrintStream;
import java.net.URL;

/**
 * NeoTerm overlay of termux-x11's CmdEntryPoint.
 *
 * The upstream file is built for a dedicated app_process; NeoTerm embeds the X
 * server in a SINGLE APK and must run it IN-PROCESS, because Android 12+ kills a
 * child app_process as a "phantom process" (SIGKILL/137) once the app goes to
 * the background. The differences from upstream are intentional and marked
 * "NeoTerm:":
 *   - static init loads libXlorie.so by name as a fallback (in-process the
 *     getResource() path resolves to a non-loadable in-APK entry) and never
 *     System.exit()s the host app;
 *   - {@link #startInProcess} starts the server inside the app process;
 *   - the constructor doesn't kill the app if start() fails in-process;
 *   - createIntent() targets our own package so the embedded MainActivity gets
 *     the ACTION_START broadcast without needing an env override.
 * Everything else mirrors upstream.
 */
@Keep @SuppressLint({"StaticFieldLeak", "UnsafeDynamicallyLoadedCode"})
public class CmdEntryPoint extends ICmdEntryInterface.Stub {
    public static final String ACTION_START = "com.termux.x11.CmdEntryPoint.ACTION_START";
    static final Handler handler;
    public static Context ctx;
    // NeoTerm: true when the server runs inside the host app's process.
    public static volatile boolean inProcess = false;
    private final Intent intent = createIntent();

    /**
     * Command-line entry point.
     *
     * @param args The command-line arguments
     */
    public static void main(String[] args) {
        android.util.Log.i("CmdEntryPoint", "commit " + BuildConfig.COMMIT);
        handler.post(() -> new CmdEntryPoint(args));
        Looper.loop();
    }

    /**
     * NeoTerm: start the X server inside the host process (single-APK embedding).
     * Called from NeoX11Service, which runs in its OWN process (android:process)
     * so this mirrors upstream's dedicated server process: the server gets a
     * process whose main thread does only X11, and the GUI MainActivity connects
     * cross-process over the ACTION_START binder. libXlorie.so is resolved from
     * nativeLibraryDir; the broadcast targets {@code appContext}'s package.
     */
    @Keep
    public static void startInProcess(Context appContext, String[] args) {
        inProcess = true;
        if (appContext != null)
            ctx = appContext.getApplicationContext();
        // Construct on this process's main looper (the server process has no UI),
        // so AChoreographer's frame callbacks are serviced by the main loop.
        handler.post(() -> {
            try {
                new CmdEntryPoint(args);
            } catch (Throwable t) {
                Log.e("CmdEntryPoint", "in-process X server failed to start", t);
            }
        });
    }

    CmdEntryPoint(String[] args) {
        if (!start(args)) {
            // NeoTerm: never kill the host app when embedded in-process.
            if (inProcess) {
                Log.e("CmdEntryPoint", "X server start(args) returned false (in-process)");
                return;
            }
            System.exit(1);
        }

        spawnListeningThread();
        sendBroadcastDelayed();
    }

    @SuppressLint({"WrongConstant", "PrivateApi"})
    private Intent createIntent() {
        String targetPackage = getenv("TERMUX_X11_OVERRIDE_PACKAGE");
        // NeoTerm: in-process, deliver to our own package (the embedded MainActivity).
        if (targetPackage == null && inProcess && ctx != null)
            targetPackage = ctx.getPackageName();
        if (targetPackage == null)
            targetPackage = "com.termux.x11";
        // We should not care about multiple instances, it should be called only by `Termux:X11` app
        // which is single instance...
        Bundle bundle = new Bundle();
        bundle.putBinder(null, this);

        Intent intent = new Intent(ACTION_START);
        intent.putExtra(null, bundle);
        intent.setPackage(targetPackage);

        if (getuid() == 0 || getuid() == 2000)
            intent.setFlags(0x00400000 /* FLAG_RECEIVER_FROM_SHELL */);

        return intent;
    }

    private void sendBroadcast() {
        sendBroadcast(intent);
    }

    static void sendBroadcast(Intent intent) {
        try {
            ctx.sendBroadcast(intent);
        } catch (Exception e) {
            if (e instanceof NullPointerException && ctx == null)
                Log.i("Broadcast", "Context is null, falling back to manual broadcasting");
            else
                Log.e("Broadcast", "Falling back to manual broadcasting, failed to broadcast intent through Context:", e);

            String packageName;
            try {
                packageName = android.app.ActivityThread.getPackageManager().getPackagesForUid(getuid())[0];
            } catch (RemoteException ex) {
                throw new RuntimeException(ex);
            }
            IActivityManager am;
            try {
                //noinspection JavaReflectionMemberAccess
                am = (IActivityManager) android.app.ActivityManager.class
                        .getMethod("getService")
                        .invoke(null);
            } catch (Exception e2) {
                try {
                    am = (IActivityManager) Class.forName("android.app.ActivityManagerNative")
                            .getMethod("getDefault")
                            .invoke(null);
                } catch (Exception e3) {
                    throw new RuntimeException(e3);
                }
            }

            assert am != null;
            IIntentSender sender = am.getIntentSender(1, packageName, null, null, 0, new Intent[] { intent },
                    null, PendingIntent.FLAG_CANCEL_CURRENT | PendingIntent.FLAG_ONE_SHOT, null, 0);
            try {
                //noinspection JavaReflectionMemberAccess
                IIntentSender.class
                        .getMethod("send", int.class, Intent.class, String.class, IBinder.class, IIntentReceiver.class, String.class, Bundle.class)
                        .invoke(sender, 0, intent, null, null, new IIntentReceiver.Stub() {
                            @Override public void performReceive(Intent i, int r, String d, Bundle e, boolean o, boolean s, int a) {}
                        }, null, null);
            } catch (Exception ex) {
                throw new RuntimeException(ex);
            }
        }
    }

    // In some cases Android Activity part can not connect opened port.
    // In this case opened port works like a lock file.
    private void sendBroadcastDelayed() {
        if (!connected())
            sendBroadcast(intent);

        handler.postDelayed(this::sendBroadcastDelayed, 1000);
    }

    void spawnListeningThread() {
        new Thread(this::listenForConnections).start();
    }

    /** @noinspection DataFlowIssue*/
    @SuppressLint("DiscouragedPrivateApi")
    public static Context createContext() {
        Context context;
        PrintStream err = System.err;
        try {
            java.lang.reflect.Field f = Class.forName("sun.misc.Unsafe").getDeclaredField("theUnsafe");
            f.setAccessible(true);
            Object unsafe = f.get(null);
            // Hiding harmless framework errors, like this:
            // java.io.FileNotFoundException: /data/system/theme_config/theme_compatibility.xml: open failed: ENOENT (No such file or directory)
            System.setErr(new PrintStream(new OutputStream() { public void write(int arg0) {} }));
            if (System.getenv("OLD_CONTEXT") != null) {
                context = android.app.ActivityThread.systemMain().getSystemContext();
            } else {
                context = ((android.app.ActivityThread) Class.
                        forName("sun.misc.Unsafe").
                        getMethod("allocateInstance", Class.class).
                        invoke(unsafe, android.app.ActivityThread.class))
                        .getSystemContext();
            }
        } catch (Exception e) {
            Log.e("Context", "Failed to instantiate context:", e);
            context = null;
        } finally {
            System.setErr(err);
        }
        return context;
    }

    public static native boolean start(String[] args);
    public native ParcelFileDescriptor getXConnection();
    public native ParcelFileDescriptor getLogcatOutput();
    private static native boolean connected();
    private native void listenForConnections();

    static {
        try {
            if (Looper.getMainLooper() == null)
                Looper.prepareMainLooper();
        } catch (Exception e) {
            Log.e("CmdEntryPoint", "Something went wrong when preparing MainLooper", e);
        }
        // NeoTerm: bind to the main looper explicitly so the class can be first
        // referenced from any thread without crashing.
        handler = new Handler(Looper.getMainLooper());
        ctx = createContext();

        boolean loaded = false;
        String path = "lib/" + Build.SUPPORTED_ABIS[0] + "/libXlorie.so";
        ClassLoader loader = CmdEntryPoint.class.getClassLoader();
        URL res = loader != null ? loader.getResource(path) : null;
        String libPath = res != null ? res.getFile().replace("file:", "") : null;
        if (libPath != null) {
            try {
                System.load(libPath);
                loaded = true;
            } catch (Throwable e) {
                Log.e("CmdEntryPoint", "Failed to dlopen " + libPath, e);
            }
        }
        if (!loaded) {
            // NeoTerm: in-process the lib is in nativeLibraryDir (and may already
            // be loaded by LorieView) — resolve it by name instead of exiting.
            try {
                System.loadLibrary("Xlorie");
                loaded = true;
            } catch (Throwable e) {
                Log.e("CmdEntryPoint", "Failed to loadLibrary Xlorie", e);
            }
        }
        if (!loaded && MainActivity.getInstance() == null) {
            System.err.println("Failed to load native library. Did you install the right apk? Try the universal one.");
            System.exit(134);
        }
    }
}

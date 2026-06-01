package io.neoterm.ui.other

import android.os.Bundle
import android.view.View
import android.widget.Button
import android.widget.RadioButton
import android.widget.RadioGroup
import android.widget.TextView
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import io.neoterm.App
import io.neoterm.R
import io.neoterm.component.config.NeoPreference
import io.neoterm.setup.ResultListener
import io.neoterm.setup.SetupHelper
import io.neoterm.setup.proot.Distro
import io.neoterm.setup.proot.ProotManager

/**
 * A proot futtatókörnyezet beállító-képernyője.
 *
 * A felhasználó egy listából (RadioGroup) választja ki a Linux-disztrót; a
 * kiválasztott disztró rootfs-ét a SetupHelper a GitHub Release-ekből tölti le
 * és bontja ki. A proot bináris már az APK-ba van csomagolva (libproot.so),
 * így csak a rootfs-t kell letölteni.
 *
 * @author kiva
 */
class SetupActivity : AppCompatActivity(), ResultListener {

  companion object {
    /** Set by "Reset App" to force a re-download even if already installed. */
    const val EXTRA_FORCE_REINSTALL = "force_reinstall"
  }

  private lateinit var distroGroup: RadioGroup
  private lateinit var installButton: Button
  private lateinit var hintText: TextView
  private var installing = false

  override fun onCreate(savedInstanceState: Bundle?) {
    super.onCreate(savedInstanceState)
    setContentView(R.layout.ui_setup)

    distroGroup = findViewById(R.id.setup_distro_group)
    installButton = findViewById(R.id.setup_install)
    hintText = findViewById(R.id.setup_distro_hint_text)

    populateDistros()

    if (intent.getBooleanExtra(EXTRA_FORCE_REINSTALL, false)) {
      installButton.setText(R.string.setup_reinstall)
    }

    installButton.setOnClickListener { startInstall() }
  }

  private fun populateDistros() {
    val current = Distro.fromId(NeoPreference.getProotDistro())
    Distro.values().forEach { distro ->
      val button = RadioButton(this).apply {
        id = View.generateViewId()
        text = distro.displayName
        tag = distro
        textSize = 16f
        setPadding(paddingLeft, dp(12), paddingRight, dp(12))
      }
      distroGroup.addView(button)
      if (distro == current) {
        distroGroup.check(button.id)
      }
    }
    if (distroGroup.checkedRadioButtonId == View.NO_ID && distroGroup.childCount > 0) {
      distroGroup.check(distroGroup.getChildAt(0).id)
    }
  }

  private fun selectedDistro(): Distro {
    val checkedId = distroGroup.checkedRadioButtonId
    val button = if (checkedId != View.NO_ID) distroGroup.findViewById<RadioButton>(checkedId) else null
    return button?.tag as? Distro ?: Distro.DEFAULT
  }

  private fun startInstall() {
    if (installing) {
      return
    }
    val distro = selectedDistro()
    NeoPreference.setProotDistro(distro.id)

    installing = true
    setInputsEnabled(false)

    // Force a re-download if launched from "Reset App", or if the chosen distro
    // is already installed (the user explicitly re-ran setup for it).
    val force = intent.getBooleanExtra(EXTRA_FORCE_REINSTALL, false) ||
      ProotManager.isInstalled(distro)

    // A SetupHelper egy folyamatjelző dialógust mutat, és a háttérszálon
    // letölti + kibontja a kiválasztott disztró rootfs-ét a release-ekből.
    SetupHelper.setupProot(
      this, this, NeoPreference.getProotSource(), distro, force
    )
  }

  override fun onResult(error: Exception?) {
    installing = false
    if (error == null) {
      setResult(RESULT_OK)
      finish()
      return
    }

    setInputsEnabled(true)
    AlertDialog.Builder(this)
      .setTitle(R.string.error)
      .setMessage(error.toString())
      .setNeutralButton(R.string.show_help) { _, _ -> App.get().openHelpLink() }
      .setNegativeButton(R.string.use_system_shell) { _, _ ->
        setResult(RESULT_CANCELED)
        finish()
      }
      .setPositiveButton(android.R.string.ok, null)
      .show()
  }

  override fun onBackPressed() {
    if (installing) {
      return
    }
    // Kilépés setup nélkül → a hívó NeoTermActivity rendszer-shellre vált.
    setResult(RESULT_CANCELED)
    super.onBackPressed()
  }

  private fun setInputsEnabled(enabled: Boolean) {
    installButton.isEnabled = enabled
    for (i in 0 until distroGroup.childCount) {
      distroGroup.getChildAt(i).isEnabled = enabled
    }
  }

  private fun dp(value: Int): Int =
    (value * resources.displayMetrics.density).toInt()
}

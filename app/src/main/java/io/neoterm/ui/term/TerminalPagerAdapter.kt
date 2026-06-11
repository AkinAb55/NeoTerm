package io.neoterm.ui.term

import android.graphics.Rect
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.FrameLayout
import androidx.core.content.ContextCompat
import androidx.recyclerview.widget.RecyclerView
import io.neoterm.NeoGLView
import io.neoterm.R
import io.neoterm.backend.TerminalColors
import io.neoterm.component.ComponentManager
import io.neoterm.component.colorscheme.ColorSchemeComponent
import io.neoterm.component.colorscheme.NeoColorScheme
import io.neoterm.component.completion.OnAutoCompleteListener
import io.neoterm.component.config.DefaultValues
import io.neoterm.component.config.NeoPreference
import io.neoterm.frontend.session.view.TerminalView
import io.neoterm.frontend.session.view.extrakey.ExtraKeysView
import io.neoterm.utils.Terminals

/**
 * Backs the [androidx.viewpager2.widget.ViewPager2] with the list of open
 * sessions. Each page is a live view bound to a session:
 *   - a terminal session -> R.layout.ui_term (TerminalView + ExtraKeysView)
 *   - an X session        -> R.layout.ui_xorg (the SDL GL surface)
 *
 * The binding reuses exactly the same [TermSessionData]/[TermTab] wiring the old
 * TabSwitcher decorator used, so a page is a fully live terminal — which (with
 * offscreenPageLimit = 1) is what lets the user peek the neighbouring tab's real
 * content while dragging.
 *
 * The model is the list of [NeoTab] (TermTab / XSessionTab) held by the
 * activity; the adapter never owns sessions, it only renders them. Stable ids
 * (the tab's identity hash) keep ViewPager2 from rebinding the wrong page after
 * insert/remove.
 */
class TerminalPagerAdapter(
  private val activity: NeoTermActivity,
  val tabs: MutableList<NeoTab>
) : RecyclerView.Adapter<TerminalPagerAdapter.PageHolder>() {

  companion object {
    private const val VIEW_TYPE_TERM = 0
    private const val VIEW_TYPE_X = 1
  }

  /** Stable id per tab, so ViewPager2 keeps each page bound to its session. */
  private val tabIds = HashMap<NeoTab, Long>()
  private var nextId = 1L

  init {
    setHasStableIds(true)
  }

  class PageHolder(val root: View) : RecyclerView.ViewHolder(root)

  private fun idFor(tab: NeoTab): Long = tabIds.getOrPut(tab) { nextId++ }

  override fun getItemId(position: Int): Long = idFor(tabs[position])

  override fun getItemCount(): Int = tabs.size

  override fun getItemViewType(position: Int): Int =
    if (tabs[position] is XSessionTab) VIEW_TYPE_X else VIEW_TYPE_TERM

  override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): PageHolder {
    val inflater = LayoutInflater.from(parent.context)
    val view = when (viewType) {
      VIEW_TYPE_X -> inflater.inflate(R.layout.ui_xorg, parent, false)
      else -> {
        val v = inflater.inflate(R.layout.ui_term, parent, false)
        val terminalView = v.findViewById<TerminalView>(R.id.terminal_view)
        val extraKeysView = v.findViewById<ExtraKeysView>(R.id.extra_keys)
        Terminals.setupTerminalView(terminalView)
        Terminals.setupExtraKeysView(extraKeysView)

        val colorSchemeManager = ComponentManager.getComponent<ColorSchemeComponent>()
        val scheme = colorSchemeManager.getCurrentColorScheme()
        colorSchemeManager.applyColorScheme(terminalView, extraKeysView, scheme)
        // Match the container background to the terminal background so swiping
        // between tabs doesn't flash a different color behind the terminal.
        v.setBackgroundColor(schemeBackgroundColor(scheme))
        v
      }
    }
    // Pages must fill the pager (ViewPager2 requires MATCH_PARENT children).
    view.layoutParams = RecyclerView.LayoutParams(
      RecyclerView.LayoutParams.MATCH_PARENT,
      RecyclerView.LayoutParams.MATCH_PARENT
    )
    return PageHolder(view)
  }

  override fun onBindViewHolder(holder: PageHolder, position: Int) {
    val tab = tabs[position]
    when (tab) {
      is TermTab -> bindTerminalPage(holder, tab)
      is XSessionTab -> bindXPage(holder, tab)
    }
  }

  private fun bindTerminalPage(holder: PageHolder, tab: TermTab) {
    tab.toolbar = activity.toolbar
    val terminalView = holder.root.findViewById<TerminalView>(R.id.terminal_view)
    val extraKeysView = holder.root.findViewById<ExtraKeysView>(R.id.extra_keys)

    val termData = tab.termData
    termData.initializeViewWith(tab, terminalView, extraKeysView)
    terminalView.setEnableWordBasedIme(termData.profile?.enableWordBasedIme ?: DefaultValues.enableWordBasedIme)
    terminalView.setTerminalViewClient(termData.viewClient)
    terminalView.attachSession(termData.termSession)

    if (NeoPreference.loadBoolean(R.string.key_general_auto_completion, false)) {
      if (termData.onAutoCompleteListener == null) {
        termData.onAutoCompleteListener = TermCompleteListener(terminalView)
      }
      terminalView.onAutoCompleteListener = termData.onAutoCompleteListener
    }

    if (termData.termSession != null) {
      termData.viewClient?.updateExtraKeys(termData.termSession?.title, true)
    }

    // Build the emulator + redraw once the page is laid out so neighbouring
    // (offscreen) pages are live and can be peeked during a drag.
    terminalView.post {
      terminalView.updateSize()
      terminalView.onScreenUpdated()
    }
  }

  private fun bindXPage(holder: PageHolder, tab: XSessionTab) {
    val sessionData = tab.sessionData ?: return
    val videoLayout = holder.root.findViewById<FrameLayout>(R.id.xorg_video_layout)

    // Re-parent the GL view onto this page's container (it may have been bound
    // to an old recycled view).
    sessionData.videoLayout = videoLayout
    videoLayout.setLayerType(View.LAYER_TYPE_NONE, null)

    val existing = sessionData.glView
    if (existing != null) {
      (existing.parent as? ViewGroup)?.removeView(existing)
      videoLayout.addView(
        existing,
        FrameLayout.LayoutParams(
          FrameLayout.LayoutParams.MATCH_PARENT,
          FrameLayout.LayoutParams.MATCH_PARENT
        )
      )
      return
    }

    Thread {
      sessionData.client?.runOnUiThread {
        val glView = NeoGLView(sessionData.client)
        sessionData.glView = glView
        glView.isFocusableInTouchMode = true
        glView.isFocusable = true
        glView.requestFocus()
        glView.setLayerType(View.LAYER_TYPE_NONE, null)
        videoLayout.addView(
          glView,
          FrameLayout.LayoutParams(
            FrameLayout.LayoutParams.MATCH_PARENT,
            FrameLayout.LayoutParams.MATCH_PARENT
          )
        )

        glView.pointerIcon = android.view.PointerIcon.getSystemIcon(
          activity, android.view.PointerIcon.TYPE_NULL
        )

        val r = Rect()
        videoLayout.getWindowVisibleDisplayFrame(r)
        glView.callNativeScreenVisibleRect(r.left, r.top, r.right, r.bottom)
        videoLayout.viewTreeObserver.addOnGlobalLayoutListener {
          val rect = Rect()
          videoLayout.getWindowVisibleDisplayFrame(rect)
          val heightDiff = videoLayout.rootView.height - videoLayout.height
          val widthDiff = videoLayout.rootView.width - videoLayout.width
          Log.v(
            "SDL",
            "Main window visible region changed: " + rect.left + ":" + rect.top + ":" + rect.width() + ":" + rect.height()
          )
          videoLayout.postDelayed({
            sessionData.glView?.callNativeScreenVisibleRect(
              rect.left + widthDiff, rect.top + heightDiff, rect.width(), rect.height()
            )
          }, 300)
          videoLayout.postDelayed({
            sessionData.glView?.callNativeScreenVisibleRect(
              rect.left + widthDiff, rect.top + heightDiff, rect.width(), rect.height()
            )
          }, 600)
        }
      }
    }.start()
  }

  override fun onViewRecycled(holder: PageHolder) {
    super.onViewRecycled(holder)
    // Detach the X GL view from a recycled page so it can be re-parented onto the
    // page it gets re-bound to (the GL surface is a single live instance).
    val videoLayout = holder.root.findViewById<FrameLayout>(R.id.xorg_video_layout) ?: return
    if (videoLayout.childCount > 0) videoLayout.removeAllViews()
  }

  /** The active color scheme's background color (falls back to the default). */
  private fun schemeBackgroundColor(scheme: NeoColorScheme): Int {
    return try {
      TerminalColors.parse(scheme.backgroundColor ?: "#14181c")
    } catch (e: Exception) {
      ContextCompat.getColor(activity, R.color.terminal_background)
    }
  }
}

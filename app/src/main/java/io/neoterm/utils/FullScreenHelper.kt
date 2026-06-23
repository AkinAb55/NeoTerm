package io.neoterm.utils

import android.view.View
import android.widget.FrameLayout
import androidx.appcompat.app.AppCompatActivity

/**
 * Helper class to "adjustResize" Activity when we are in full screen mode and check IME status.
 * Android Bug 5497: https://code.google.com/p/android/issues/detail?id=5497
 */
class FullScreenHelper private constructor(
  activity: AppCompatActivity,
  var fullScreen: Boolean,
  private var shouldSkipFirst: Boolean
) {

  interface KeyBoardListener {
    /**
     * call back

     * @param isShow         true is show else hidden
     * *
     * @param keyboardHeight screenKeyboard height
     */
    fun onKeyboardChange(isShow: Boolean, keyboardHeight: Int)
  }

  private val mChildOfContent: View

  private var mOriginHeight: Int = 0
  private var mPreHeight: Int = 0
  private var mKeyBoardListener: KeyBoardListener? = null

  fun setKeyBoardListener(mKeyBoardListener: KeyBoardListener) {
    this.mKeyBoardListener = mKeyBoardListener
  }

  init {
    val content = activity.findViewById<FrameLayout>(android.R.id.content)
    mChildOfContent = content.getChildAt(0)
    mChildOfContent.viewTreeObserver.addOnGlobalLayoutListener {
      // NOTE: the old AndroidBug5497 workaround (possiblyResizeChildOfContent) used to force a
      // fixed pixel height on the content view while in full screen. The terminal now handles the
      // soft keyboard itself, by panning content against the window insets (see the activity's
      // OnApplyWindowInsetsListener), so that manual resize is redundant *and* conflicts with the
      // inset-based layout — in full screen it collapsed the content to a fixed (often zero/black)
      // height. Leave the content at its XML match_parent height and only keep IME monitoring,
      // which still drives the toolbar auto-hide.
      monitorImeStatus()
    }
  }

  private fun monitorImeStatus() {
    val currHeight = mChildOfContent.height
    if (currHeight == 0 && shouldSkipFirst) {
      // First time
      return
    }

    shouldSkipFirst = false
    var hasChange = false
    if (mPreHeight == 0) {
      mPreHeight = currHeight
      mOriginHeight = currHeight
    } else {
      if (mPreHeight != currHeight) {
        hasChange = true
        mPreHeight = currHeight
      } else {
        hasChange = false
      }
    }
    if (hasChange) {
      var keyboardHeight = 0
      val keyBoardIsShowing: Boolean
      if (Math.abs(mOriginHeight - currHeight) < 100) {
        //hidden
        keyBoardIsShowing = false
      } else {
        //show
        keyboardHeight = mOriginHeight - currHeight
        keyBoardIsShowing = true
      }

      if (mKeyBoardListener != null) {
        mKeyBoardListener!!.onKeyboardChange(keyBoardIsShowing, keyboardHeight)
      }
    }
  }

  companion object {
    fun injectActivity(activity: AppCompatActivity, fullScreen: Boolean, recreate: Boolean): FullScreenHelper {
      return FullScreenHelper(activity, fullScreen, recreate)
    }

  }
}
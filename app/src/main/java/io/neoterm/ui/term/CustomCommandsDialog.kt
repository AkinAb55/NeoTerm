package io.neoterm.ui.term

import android.content.Context
import android.text.InputType
import android.text.TextUtils
import android.util.TypedValue
import android.view.Gravity
import android.view.View
import android.view.ViewGroup
import android.widget.EditText
import android.widget.LinearLayout
import android.widget.TextView
import androidx.appcompat.app.AlertDialog
import androidx.recyclerview.widget.ItemTouchHelper
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import io.neoterm.R
import io.neoterm.utils.CustomCommands
import java.util.Collections

/**
 * The "CC" (custom commands) manager, opened from the terminal's text-selection
 * bar. Lets the user add / edit / delete and reorder named shell commands, and
 * run one either in the current session (→) or in a freshly opened one (⇉).
 *
 * Reordering is by long-press + drag (ItemTouchHelper). Tapping a name edits it.
 *
 * @param runInCurrent runs the command line in the active session
 * @param runInNew     opens a new session that runs the command line
 */
class CustomCommandsDialog(
  private val context: Context,
  private val runInCurrent: (String) -> Unit,
  private val runInNew: (String) -> Unit
) {
  private val commands = CustomCommands.load(context)
  private val adapter = Adapter()
  private lateinit var emptyHint: TextView
  private var mainDialog: AlertDialog? = null

  private fun dp(v: Int) = TypedValue.applyDimension(
    TypedValue.COMPLEX_UNIT_DIP, v.toFloat(), context.resources.displayMetrics
  ).toInt()

  fun show() {
    emptyHint = TextView(context).apply {
      setText(R.string.cc_empty)
      setPadding(dp(8), dp(16), dp(8), dp(16))
    }
    // Cap the list height so long lists scroll instead of overflowing the dialog.
    val recycler = object : RecyclerView(context) {
      override fun onMeasure(widthSpec: Int, heightSpec: Int) {
        val maxH = (resources.displayMetrics.heightPixels * 0.6f).toInt()
        super.onMeasure(widthSpec, MeasureSpec.makeMeasureSpec(maxH, MeasureSpec.AT_MOST))
      }
    }.apply {
      layoutManager = LinearLayoutManager(context)
      this.adapter = this@CustomCommandsDialog.adapter
    }
    ItemTouchHelper(DragCallback()).attachToRecyclerView(recycler)

    val container = LinearLayout(context).apply {
      orientation = LinearLayout.VERTICAL
      setPadding(dp(8), dp(4), dp(8), dp(4))
      addView(emptyHint)
      addView(recycler)
    }
    updateEmpty()

    val dialog = AlertDialog.Builder(context)
      .setTitle(R.string.custom_commands)
      // Keep the manager open after Add (listener overridden below).
      .setNeutralButton(R.string.cc_add, null)
      .setNegativeButton(R.string.cc_close, null)
      .setView(container)
      .create()
    dialog.setOnShowListener {
      dialog.getButton(AlertDialog.BUTTON_NEUTRAL).setOnClickListener { editOrAdd(null) }
    }
    mainDialog = dialog
    dialog.show()
  }

  private fun updateEmpty() {
    emptyHint.visibility = if (commands.isEmpty()) View.VISIBLE else View.GONE
  }

  private fun glyph(text: String, desc: Int): TextView = TextView(context).apply {
    this.text = text
    contentDescription = context.getString(desc)
    textSize = 20f
    gravity = Gravity.CENTER
    minWidth = dp(44)
    setPadding(dp(6), dp(10), dp(6), dp(10))
    isClickable = true
  }

  private inner class VH(row: LinearLayout) : RecyclerView.ViewHolder(row) {
    val name: TextView = row.getChildAt(0) as TextView
    val runCur: TextView = row.getChildAt(1) as TextView
    val runNew: TextView = row.getChildAt(2) as TextView
    val delete: TextView = row.getChildAt(3) as TextView
  }

  private inner class Adapter : RecyclerView.Adapter<VH>() {
    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): VH {
      val name = TextView(context).apply {
        textSize = 16f
        isSingleLine = true
        ellipsize = TextUtils.TruncateAt.END
        setPadding(dp(4), dp(12), dp(8), dp(12))
        isClickable = true
        layoutParams = LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1f)
      }
      val row = LinearLayout(context).apply {
        orientation = LinearLayout.HORIZONTAL
        gravity = Gravity.CENTER_VERTICAL
        layoutParams = RecyclerView.LayoutParams(
          ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT
        )
        addView(name)
        addView(glyph("→", R.string.cc_run_current))
        addView(glyph("⇉", R.string.cc_run_new))
        addView(glyph("✕", R.string.cc_delete))
      }
      return VH(row)
    }

    override fun onBindViewHolder(holder: VH, position: Int) {
      val cmd = commands[position]
      holder.name.text = cmd.name
      // Tap the name to edit; long-press anywhere on the row starts the drag.
      holder.name.setOnClickListener {
        val p = holder.adapterPosition
        if (p != RecyclerView.NO_POSITION) editOrAdd(commands[p])
      }
      holder.runCur.setOnClickListener {
        val p = holder.adapterPosition
        if (p != RecyclerView.NO_POSITION) {
          runInCurrent(commands[p].command)
          mainDialog?.dismiss()
        }
      }
      holder.runNew.setOnClickListener {
        val p = holder.adapterPosition
        if (p != RecyclerView.NO_POSITION) {
          runInNew(commands[p].command)
          mainDialog?.dismiss()
        }
      }
      holder.delete.setOnClickListener {
        val p = holder.adapterPosition
        if (p != RecyclerView.NO_POSITION) {
          commands.removeAt(p)
          notifyItemRemoved(p)
          CustomCommands.save(context, commands)
          updateEmpty()
        }
      }
    }

    override fun getItemCount() = commands.size
  }

  private inner class DragCallback : ItemTouchHelper.SimpleCallback(
    ItemTouchHelper.UP or ItemTouchHelper.DOWN, 0
  ) {
    override fun onMove(
      recyclerView: RecyclerView, viewHolder: RecyclerView.ViewHolder,
      target: RecyclerView.ViewHolder
    ): Boolean {
      val from = viewHolder.adapterPosition
      val to = target.adapterPosition
      if (from == RecyclerView.NO_POSITION || to == RecyclerView.NO_POSITION) return false
      Collections.swap(commands, from, to)
      adapter.notifyItemMoved(from, to)
      return true
    }

    override fun onSwiped(viewHolder: RecyclerView.ViewHolder, direction: Int) {}

    override fun isLongPressDragEnabled() = true

    override fun clearView(recyclerView: RecyclerView, viewHolder: RecyclerView.ViewHolder) {
      super.clearView(recyclerView, viewHolder)
      // Persist the new order once the drag settles.
      CustomCommands.save(context, commands)
    }
  }

  private fun editOrAdd(existing: CustomCommands.Cmd?) {
    val nameEdit = EditText(context).apply {
      hint = context.getString(R.string.cc_name_hint)
      setText(existing?.name ?: "")
      inputType = InputType.TYPE_CLASS_TEXT
    }
    val cmdEdit = EditText(context).apply {
      hint = context.getString(R.string.cc_command_hint)
      setText(existing?.command ?: "")
      inputType = InputType.TYPE_CLASS_TEXT or InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS
    }
    val layout = LinearLayout(context).apply {
      orientation = LinearLayout.VERTICAL
      setPadding(dp(20), dp(8), dp(20), 0)
      addView(nameEdit)
      addView(cmdEdit)
    }
    AlertDialog.Builder(context)
      .setTitle(if (existing == null) R.string.cc_add else R.string.cc_edit)
      .setView(layout)
      .setPositiveButton(android.R.string.ok) { _, _ ->
        val command = cmdEdit.text.toString().trim()
        if (command.isEmpty()) return@setPositiveButton
        val name = nameEdit.text.toString().trim().ifEmpty { command }
        if (existing == null) {
          commands.add(CustomCommands.Cmd(name, command))
          adapter.notifyItemInserted(commands.size - 1)
        } else {
          existing.name = name
          existing.command = command
          val idx = commands.indexOf(existing)
          if (idx >= 0) adapter.notifyItemChanged(idx)
        }
        CustomCommands.save(context, commands)
        updateEmpty()
      }
      .setNegativeButton(android.R.string.cancel, null)
      .show()
  }
}

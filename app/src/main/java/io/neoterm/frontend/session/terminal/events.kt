package io.neoterm.frontend.session.terminal

import io.neoterm.ui.term.TermTab

class CreateNewSessionEvent(val initialCommand: String? = null)
class SwitchIndexedSessionEvent(val index: Int)
class SwitchSessionEvent(val toNext: Boolean)
class TabCloseEvent(val termTab: TermTab)
class TitleChangedEvent(val title: String)
class ToggleFullScreenEvent
class ToggleImeEvent
class FontSizeChangedEvent(val fontSize: Int)

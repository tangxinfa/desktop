# -*- coding: utf-8 -*-

import re
from xkeysnail.transform import *

# Emulate hhkb keyboard layout:
#   Switch left win and left control
#   Set capslock as left control
define_conditional_modmap(lambda wm_class, device_name: device_name != "Topre Corporation HHKB Professional", {
    Key.LEFT_META: Key.LEFT_ALT,
    Key.LEFT_ALT: Key.LEFT_META,
    Key.LEFT_CTRL: Key.LEFT_ALT,
    Key.CAPSLOCK: Key.LEFT_CTRL,
})

# Keybindings for Firefox/Chrome
define_keymap(re.compile("Firefox|Google-chrome|Chromium"), {
    # Type C-j to focus to the content
    K("C-j"): K("C-f6"),
}, "Firefox and Chrome")

# Keybindings for Firefox
define_keymap(re.compile("Firefox"), {
    # Ctrl+C+o+l to store link by firefox addon:
    #     https://addons.mozilla.org/en-US/firefox/addon/titleurlcopy/?src=userprofile
    K("C-c"): {
        K("o"): {
            K("l"): [K("C-c")]
        },
        K("r"): {
            K("b"): [K("C-Shift-o")]
        },
        K("C-n"): K("C-TAB"),
        K("C-p"): K("C-Shift-TAB"),
        K("C-b"): K("M-left"),
        K("C-f"): K("M-right"),
        K("b"): {
            K("u"): [K("F5")]
        },
    }
}, "Firefox")

# Keybindings for Chrome
define_keymap(re.compile("Google-chrome|Chromium"), {
    # Ctrl+C+o+l to store link by chrome addon:
    #     https://github.com/cage1016/copy-2-clipboard-with-ease
    #     set extension shortcut to: Alt-c
    K("C-c"): {
        K("o"): {
            K("l"): K("M-c")
        },
        K("C-n"): K("C-TAB"),
        K("C-p"): K("C-Shift-TAB"),
        K("b"): {
            K("u"): [K("F5")]
        },
    },
}, "Chrome")

# Keybindings for Zeal
define_keymap(re.compile("Zeal"), {
    # Type Esc or C-g to quit
    K("C-g"): K("C-q"),
    K("esc"): K("C-q"),
}, "Zeal")

# Keybindings for keepassxc
define_keymap(re.compile("keepassxc"), {
    # Type C-g to move to scratchpad
    # Type C-c c n to copy username
    # Type C-c c p to copy password
    # Type C-c c o to open url
    K("C-g"): K("Super-Minus"),
    K("C-c"): {
        K("c"): {
            K("n"): [K("C-b"), K("Super-Minus")],
            K("p"): [K("C-c"), K("Super-Minus")],
        },
        K("C-o"): [K("C-U"), K("Super-Minus")],
    }
}, "keepassxc")

# Keybindings for dingtalk
define_keymap(re.compile("dingtalk"), {
    # Type C-c o s to capture screenshot
    K("C-c"): {
        K("o"): {
            K("s"): [K("C-M-a")],
        },
    }
}, "dingtalk")

# Emacs-like keybindings in non-Emacs applications
define_keymap(lambda wm_class: wm_class not in ("Emacs", "URxvt", "Rofi", "Gnome-terminal", "Chinese-calendar", "netease-cloud-music", "VirtualBox Machine", ""), {
    # Cursor
    K("C-b"): with_mark(K("left")),
    K("C-f"): with_mark(K("right")),
    K("C-p"): with_mark(K("up")),
    K("C-n"): with_mark(K("down")),
    K("C-h"): with_mark(K("backspace")),
    # Forward/Backward word
    K("M-b"): with_mark(K("C-left")),
    K("M-f"): with_mark(K("C-right")),
    # Beginning/End of line
    K("C-a"): with_mark(K("home")),
    K("C-e"): with_mark(K("end")),
    # Page up/down
    K("M-v"): with_mark(K("page_up")),
    K("C-v"): with_mark(K("page_down")),
    # Beginning/End of file
    K("M-Shift-comma"): with_mark(K("C-home")),
    K("M-Shift-dot"): with_mark(K("C-end")),
    # Newline
    K("C-m"): K("enter"),
    K("C-j"): K("enter"),
    K("C-o"): [K("enter"), K("left")],
    # Copy
    K("C-w"): [K("C-x"), set_mark(False)],
    K("M-w"): [K("C-c"), set_mark(False)],
    K("C-y"): [K("C-v"), set_mark(False)],
    # Delete
    K("C-d"): [K("delete"), set_mark(False)],
    K("M-d"): [K("C-delete"), set_mark(False)],
    # Kill line
    K("C-k"): [K("Shift-end"), K("C-x"), K("delete"), set_mark(False)],
    # Undo
    K("C-slash"): [K("C-z"), set_mark(False)],
    K("C-Shift-ro"): K("C-z"),
    # Mark
    K("C-Shift-key_2"): set_mark(True),
    # Search
    K("C-s"): K("F3"),
    K("C-r"): K("Shift-F3"),
    K("M-Shift-key_5"): K("C-h"),
    # Cancel
    K("C-g"): [K("esc"), set_mark(False)],
    # Escape
    K("C-q"): escape_next_key,
    # C-x YYY
    K("C-x"): {
        # C-x h (select all)
        K("h"): [K("C-home"), K("C-a"), set_mark(True)],
        # C-x C-f (open)
        K("C-f"): K("C-o"),
        # C-x C-s (save)
        K("C-s"): K("C-s"),
        # C-x k (kill tab)
        K("k"): K("C-f4"),
        # C-x C-c (exit)
        K("C-c"): K("C-q"),
        # cancel
        K("C-g"): pass_through_key,
        # C-x u (undo)
        K("u"): [K("C-z"), set_mark(False)],
        K("o"): {
            # C-x o g (Focus search bar)
            K("g"): K("C-k"),
            # C-x o s (Focus find bar)
            K("s"): K("C-f"),
            # C-x o l (Focus address bar)
            K("l"): K("C-l"),
            # C-x o . (Focus content area)
            K("dot"): [K("C-k"), K("C-g"), K("TAB"), set_mark(False)],
        },
        K("r"): {
            # C-x r m (Set a bookmark)
            K("m"): K("C-d"),
            # C-x r l (List bookmarks)
            K("l"): K("C-Shift-o"),
        },
    }
}, "Emacs-like keys")

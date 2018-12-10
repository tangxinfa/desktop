#
# ~/.bash_profile
#

[[ -f ~/.profile ]] && . ~/.profile

if [ ! -z "$XDG_SESSION_DESKTOP" ]; then
    # A desktop manager already running.
    return
fi

if [ -z "$DISPLAY" ] && [ -n "$XDG_VTNR" ] && [ "$XDG_VTNR" -eq 1 ]; then
    # Choice i3wm or emacs or fbterm on tty1.
    CHOICE=$(dialog --clear --title "Launcher" --no-lines\
                    --menu "" 10 79 4 \
                    "i3wm" ""\
                    "emacs" ""\
                    "fbterm" ""\
                    2>&1 >/dev/tty)
    clear
    case $CHOICE in
        i3wm) exec startx; return;;
        emacs) ;;
        fbterm) ;;
        *) return;;
    esac
fi


# Enable keymaps if xkeysnail not running.
if [ "`ps waux | grep 'python* /usr/bin/xkeysnail' | grep -v grep | wc -l`" -eq "0" ]; then
    echo "Keymaps ... [ON]" >/dev/stderr
    ~/bin/keymaps on
else
    echo "Keymaps ... [OFF]" >/dev/stderr
    ~/bin/keymaps off
fi

# Startup fbterm on tty.
FBTERM=/usr/bin/fbterm
if [ ! -f ${FBTERM} ]; then
    echo "${FBTERM} not exists"
    return
fi

# Startup fcitx which required by fcitx-fbterm.
pidof fcitx >/dev/null 2>&1
if [ "$?" -ne "0" ]; then
    echo "Startup fcitx" >/dev/stderr
    fcitx -d >/dev/null 2>&1
fi

if [ -e /usr/share/terminfo/x/xterm-256color ]; then
    export TERM=xterm-256color
fi

ls -l ${FBTERM} | awk '{print $1}' | grep s >/dev/null 2>&1
if [ "$?" -ne "0" ]; then
    echo "Setting allow non-root user execute ${FBTERM} with root permission ..." >/dev/stderr
    sudo chmod u+s ${FBTERM}
    if [ "$?" -eq "0" ]; then
        echo "Setting allow non-root user execute ${FBTERM} with root permission ... [OK]" >/dev/stderr
    else
        echo "Setting allow non-root user execute ${FBTERM} with root permission ... [FAILED]" >/dev/stderr
    fi
fi

getcap ${FBTERM} | grep -F 'cap_sys_tty_config+ep' >/dev/null 2>&1
if [ "$?" -ne "0" ]; then
    echo "Setting allow ${FBTERM} set system shortcuts ..." >/dev/stderr
    sudo setcap 'cap_sys_tty_config+ep' ${FBTERM}
    if [ "$?" -eq "0" ]; then
        echo "Setting allow ${FBTERM} set system shortcuts ... [OK]" >/dev/stderr
    else
        echo "Setting allow ${FBTERM} set system shortcuts ... [FAILED]" >/dev/stderr
    fi
fi

if [ "$CHOICE" = "emacs" ]; then
    echo "Startup emacs" >/dev/stderr
    clear
    PROGRAM="emacs -nw --color=no --no-x-resources"
else
    WALLPAPER="`cat ~/.config/variety/wallpaper/wallpaper.jpg.txt 2>/dev/null`"
    if [ -x /usr/bin/fbv ] && [ "$WALLPAPER" != "" ]; then
        echo "Enable wallpaper ${WALLPAPER}" > /dev/stderr
        echo -ne "\e[?25l" # hide cursor
        /usr/bin/fbv -ciuker "$WALLPAPER" << EOF
q
EOF
        shift
        export FBTERM_BACKGROUND_IMAGE=1
    fi
fi

exec ${FBTERM} -- $PROGRAM

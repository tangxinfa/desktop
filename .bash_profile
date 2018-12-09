#
# ~/.bash_profile
#

[[ -f ~/.profile ]] && . ~/.profile

if [ -z "$DISPLAY" ] && [ -n "$XDG_VTNR" ] && [ "$XDG_VTNR" -eq 1 ]; then
    # TTY1 for graphical desktop, startup i3wm directly if graphical mode disabled.
    if [ "multi-user.target" = "`systemctl get-default`" ]; then
        exec startx
    fi
else
    echo "Setting Caps-Lock as Control ..." >/dev/stderr
    ~/bin/keymaps on
    if [ "$?" -eq "0" ]; then
        echo "Setting Caps-Lock as Control ... [OK]" >/dev/stderr
    else
        echo "Setting Caps-Lock as Control ... [FAILED]" >/dev/stderr
    fi

    # Other TTY for console desktop, startup fbterm.
    FBTERM=/usr/bin/fbterm
    if [ -f ${FBTERM} ]; then
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
        pidof fcitx >/dev/null 2>&1
        if [ "$?" -ne "0" ]; then
            echo "Startup fcitx" >/dev/stderr
            fcitx -d
        fi
        WALLPAPER="`cat ~/.config/variety/wallpaper/wallpaper.jpg.txt`"
        if [ -x /usr/bin/fbv ] && [ "$WALLPAPER" != "" ]; then
            echo "Enable wallpaper ${WALLPAPER}" > /dev/stderr
            echo -ne "\e[?25l" # hide cursor
            /usr/bin/fbv -ciuker "$WALLPAPER" << EOF
q
EOF
            shift
            export FBTERM_BACKGROUND_IMAGE=1
        fi
        pidof emacs >/dev/null 2>&1
        if [ "$?" -ne "0" ] && [ "$XDG_VTNR" -eq 2 ] ; then
            # TTY2 dedicated to running "emacs" if emacs not running
            echo "Startup emacs" >/dev/stderr
            clear
            PROGRAM="emacs -nw --color=no"
        fi
        export TERM=fbterm
        exec ${FBTERM} -- $PROGRAM
    fi
fi

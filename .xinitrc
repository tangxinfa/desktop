[ -f /etc/xprofile ] && source /etc/xprofile
[ -f ~/.xprofile ] && source ~/.xprofile

if [ "graphical.target" != "`systemctl get-default`" ]; then
    export XDG_CURRENT_DESKTOP=i3
    exec dbus-launch i3
fi

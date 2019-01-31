[ -f /etc/xprofile ] && source /etc/xprofile
[ -f ~/.xprofile ] && source ~/.xprofile

if [ "graphical.target" != "`systemctl get-default`" ]; then
    exec dbus-launch i3
fi

[ -f /etc/xprofile ] && source /etc/xprofile
[ -f ~/.xprofile ] && source ~/.xprofile

export XDG_CURRENT_DESKTOP=i3
exec dbus-launch i3

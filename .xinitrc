if [ -d /etc/X11/xinit/xinitrc.d ] ; then
 for f in /etc/X11/xinit/xinitrc.d/?*.sh ; do
  [ -x "$f" ] && . "$f"
 done
 unset f
fi

[[ -f /etc/X11/xinit/.Xresources ]] && xrdb -merge /etc/X11/xinit/.Xresources

[ -f /etc/xprofile ] && source /etc/xprofile
[ -f ~/.xprofile ] && source ~/.xprofile

if [ "graphical.target" != "`systemctl get-default`" ]; then
    ~/bin/dual-screen-detect
    [[ -f ~/.config/X11/xresources ]] && xrdb -merge ~/.config/X11/xresources
    eval $(dbus-launch --sh-syntax)
    exec i3
fi

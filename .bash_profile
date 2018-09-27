#
# ~/.bash_profile
#

[[ -f ~/.profile ]] && . ~/.profile

if [ -z "$DISPLAY" ] && [ -n "$XDG_VTNR" ] && [ "$XDG_VTNR" -eq 1 ]; then
  exec startx
fi

#
# ~/.bash_profile
#

[[ -f ~/.profile ]] && . ~/.profile

setterm --blank 15

if [[ -z $DISPLAY ]] &&\
   [[ -z $XDG_SESSION_TYPE || $XDG_SESSION_TYPE = tty ]] &&\
   [[ $(tty) = /dev/tty1 || $(tty) = /dev/tty2 ]]; then
    . ~/.launcher
fi

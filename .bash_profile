#
# ~/.bash_profile
#

[[ -f ~/.profile ]] && . ~/.profile

if [[ -z $DISPLAY ]] &&\
   [[ -z $XDG_SESSION_TYPE ]] &&\
   [[ $(tty) = /dev/tty1 || $(tty) = /dev/tty2 ]]; then
    . ~/.launcher
fi

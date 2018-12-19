#
# ~/.bash_profile
#

[[ -f ~/.profile ]] && . ~/.profile

if [[ -z $DISPLAY ]] && [[ $(tty) = /dev/tty1 ]] && [[ -z $XDG_SESSION_TYPE ]]; then
    . ~/.launcher
fi

.PHONY: all build install

EMACS_LISP_DIR=$(shell emacs --batch --eval '(message "%s" (car load-path))' 2>&1)

build: ${CURDIR}/bin/keymap-ctl ${CURDIR}/bin/xkeysnail-ctl ${CURDIR}/bin/tty-ctl ${CURDIR}/bin/file-open ${CURDIR}/bin/fcitx-fbterm-launcher

install: build
	sudo setcap 'cap_sys_tty_config+ep' `which fbterm`
	sudo chown root:root ${CURDIR}/bin/keymap-ctl && sudo chmod u+s ${CURDIR}/bin/keymap-ctl
	sudo chown root:root ${CURDIR}/bin/xkeysnail-ctl && sudo chmod u+s ${CURDIR}/bin/xkeysnail-ctl
	sudo chown root:root ${CURDIR}/bin/tty-ctl && sudo chmod u+s ${CURDIR}/bin/tty-ctl
	sudo chown root:root ${CURDIR}/bin/file-open && sudo chmod u+s ${CURDIR}/bin/file-open
	sudo chown root:root ${CURDIR}/bin/fcitx-fbterm-launcher && sudo chmod u+s ${CURDIR}/bin/fcitx-fbterm-launcher
	@echo "Install by create symbol links ..."
	-cp -rs ${CURDIR}/{bin,.config,.xinitrc,.xprofile,.fbtermrc,.launcher} ~/
	-sudo cp -rs ${CURDIR}/lisp/* ${EMACS_LISP_DIR}/

$(CURDIR)/bin/keymap-ctl: $(CURDIR)/src/keymap-ctl.c
	gcc -g -O0 ${CURDIR}/src/keymap-ctl.c -lm -o ${CURDIR}/bin/keymap-ctl

$(CURDIR)/bin/xkeysnail-ctl: $(CURDIR)/src/xkeysnail-ctl.c
	gcc -g -O0 ${CURDIR}/src/xkeysnail-ctl.c -lm -o ${CURDIR}/bin/xkeysnail-ctl

$(CURDIR)/bin/tty-ctl: $(CURDIR)/src/tty-ctl.c
	gcc -g -O0 ${CURDIR}/src/tty-ctl.c -lm -o ${CURDIR}/bin/tty-ctl

$(CURDIR)/bin/file-open: $(CURDIR)/src/file-open.c
	gcc -g -O0 ${CURDIR}/src/file-open.c -lm -o ${CURDIR}/bin/file-open

$(CURDIR)/bin/fcitx-fbterm-launcher: $(CURDIR)/src/fcitx-fbterm-launcher.c
	gcc -g -O0 ${CURDIR}/src/fcitx-fbterm-launcher.c -lm -o ${CURDIR}/bin/fcitx-fbterm-launcher

clean:
	-rm -f ${CURDIR}/bin/{keymap-ctl,xkeysnail-ctl,tty-ctl,file-open,fcitx-fbterm-launcher}

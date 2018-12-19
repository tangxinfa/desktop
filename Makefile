.PHONY: all build install

EMACS_LISP_DIR=$(shell emacs --batch --eval '(message "%s" (car load-path))' 2>&1)

build: ${CURDIR}/bin/keymap-ctl ${CURDIR}/bin/xkeysnail-ctl ${CURDIR}/bin/tty-ctl ${CURDIR}/bin/fbviewer

install: build
	sudo setcap 'cap_sys_tty_config+ep' `which fbterm`
	sudo chown root:root ${CURDIR}/bin/keymap-ctl && sudo chmod u+s ${CURDIR}/bin/keymap-ctl
	sudo chown root:root ${CURDIR}/bin/xkeysnail-ctl && sudo chmod u+s ${CURDIR}/bin/xkeysnail-ctl
	sudo chown root:root ${CURDIR}/bin/tty-ctl && sudo chmod u+s ${CURDIR}/bin/tty-ctl
	sudo chown root:root ${CURDIR}/bin/fbviewer && sudo chmod u+s ${CURDIR}/bin/fbviewer
	@echo "Install by create symbol links ..."
	-cp -rs ${CURDIR}/{bin,.config,.xinitrc,.fbtermrc,.launcher} ~/
	-sudo cp -rs ${CURDIR}/lisp/* ${EMACS_LISP_DIR}/

$(CURDIR)/bin/keymap-ctl: $(CURDIR)/src/keymap-ctl.c
	gcc -g -O0 ${CURDIR}/src/keymap-ctl.c -lm -o ${CURDIR}/bin/keymap-ctl

$(CURDIR)/bin/xkeysnail-ctl: $(CURDIR)/src/xkeysnail-ctl.c
	gcc -g -O0 ${CURDIR}/src/xkeysnail-ctl.c -lm -o ${CURDIR}/bin/xkeysnail-ctl

$(CURDIR)/bin/tty-ctl: $(CURDIR)/src/tty-ctl.c
	gcc -g -O0 ${CURDIR}/src/tty-ctl.c -lm -o ${CURDIR}/bin/tty-ctl

$(CURDIR)/bin/fbviewer: $(CURDIR)/src/fbviewer.c
	gcc -g -O0 ${CURDIR}/src/fbviewer.c -lm -o ${CURDIR}/bin/fbviewer

clean:
	-rm -f ${CURDIR}/bin/{keymap-ctl,xkeysnail-ctl,tty-ctl,fbviewer}

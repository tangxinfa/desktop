.PHONY: all build install

build: ${CURDIR}/bin/keymap-ctl ${CURDIR}/bin/xkeysnail-ctl ${CURDIR}/bin/tty-ctl ${CURDIR}/bin/file-open ${CURDIR}/bin/lid-monitor ${CURDIR}/bin/bluetooth-mobile-ctl ${CURDIR}/bin/i3lock

install: build
	sudo setcap 'cap_sys_tty_config+ep' `which fbterm`
	sudo chown root:root ${CURDIR}/bin/keymap-ctl && sudo chmod gu+s ${CURDIR}/bin/keymap-ctl
	sudo chown root:root ${CURDIR}/bin/xkeysnail-ctl && sudo chmod gu+s ${CURDIR}/bin/xkeysnail-ctl
	sudo chown root:root ${CURDIR}/bin/tty-ctl && sudo chmod gu+s ${CURDIR}/bin/tty-ctl
	sudo chown root:root ${CURDIR}/bin/file-open && sudo chmod gu+s ${CURDIR}/bin/file-open
	sudo chown root:root ${CURDIR}/bin/lid-monitor && sudo chmod gu+s ${CURDIR}/bin/lid-monitor
	sudo chown root:root ${CURDIR}/bin/bluetooth-mobile-ctl && sudo chmod gu+s ${CURDIR}/bin/bluetooth-mobile-ctl
	sudo cp ${CURDIR}/service/desktop-lock@.service /etc/systemd/system/
	sudo systemctl daemon-reload
	sudo systemctl enable desktop-lock@${USER}.service
	@echo "Install by create symbol links ..."
	-cp -rs ${CURDIR}/{bin,.config,.local,.xinitrc,.xprofile,.fbtermrc,.launcher,.mlterm} ~/

$(CURDIR)/bin/keymap-ctl: $(CURDIR)/src/keymap-ctl.c
	gcc -g -O0 ${CURDIR}/src/keymap-ctl.c -lm -o ${CURDIR}/bin/keymap-ctl

$(CURDIR)/bin/xkeysnail-ctl: $(CURDIR)/src/xkeysnail-ctl.c
	gcc -g -O0 ${CURDIR}/src/xkeysnail-ctl.c -lm -o ${CURDIR}/bin/xkeysnail-ctl

$(CURDIR)/bin/tty-ctl: $(CURDIR)/src/tty-ctl.c
	gcc -g -O0 ${CURDIR}/src/tty-ctl.c -lm -o ${CURDIR}/bin/tty-ctl

$(CURDIR)/bin/file-open: $(CURDIR)/src/file-open.c
	gcc -g -O0 ${CURDIR}/src/file-open.c -lm -o ${CURDIR}/bin/file-open

$(CURDIR)/bin/lid-monitor: $(CURDIR)/src/lid-monitor.c
	gcc -g -O0 ${CURDIR}/src/lid-monitor.c -lm -o ${CURDIR}/bin/lid-monitor

$(CURDIR)/bin/bluetooth-mobile-ctl: $(CURDIR)/src/bluetooth-mobile-ctl.c
	gcc -g -O0 ${CURDIR}/src/bluetooth-mobile-ctl.c -lm -o ${CURDIR}/bin/bluetooth-mobile-ctl

$(CURDIR)/bin/i3lock: $(CURDIR)/i3lock
	cd ${CURDIR}/i3lock &&\
	autoreconf -fi &&\
	mkdir -p build && cd build &&\
	../configure --prefix=${CURDIR} &&\
	make install

$(CURDIR)/i3lock:
	git clone https://github.com/Lixxia/i3lock.git

clean:
	-rm -rf ${CURDIR}/i3lock
	-rm -f ${CURDIR}/bin/{keymap-ctl,xkeysnail-ctl,tty-ctl,file-open,lid-monitor,bluetooth-mobile-ctl,i3lock}

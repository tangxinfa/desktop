.PHONY: all build install

build: ${CURDIR}/bin/keymaps

install: build
	@echo "Allow non-root user execute ${CURDIR}/bin/keymaps with root permission ..."
	sudo chown root:root ${CURDIR}/bin/keymaps && sudo chmod u+s ${CURDIR}/bin/keymaps
	@echo "Install by create symbol links ..."
	-cp -rs ${CURDIR}/{bin,.config,.emacs.d,.xinitrc,.fbtermrc} ~/

$(CURDIR)/bin/keymaps: $(CURDIR)/src/keymaps.c
	gcc -g -O0 ${CURDIR}/src/keymaps.c -lm -o ${CURDIR}/bin/keymaps

clean:
	-rm -f ${CURDIR}/bin/keymaps

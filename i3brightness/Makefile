all: i3brightness

i3brightness: i3brightness.c
	gcc -o $@ $^

install: i3brightness
	cp ./i3brightness ../bin/ &&\
	chown root:root ../bin/i3brightness &&\
	chmod 4755 ../bin/i3brightness

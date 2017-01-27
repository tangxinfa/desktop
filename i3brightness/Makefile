all: i3brightness

i3brightness: i3brightness.c
	gcc -o $@ $^

install: i3brightness
	cp i3brightness ../bin/ &&\
	chown root:root ./i3brightness &&\
	chmod 4755 ./i3brightness

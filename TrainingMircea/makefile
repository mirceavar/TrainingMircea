CC=gcc

PATHS = -I/usr/include/glib-2.0 \
		-I/usr/include/dbus-1.0 \
		-I/usr/include/gio-unix-2.0 \
		-I/usr/lib/x86_64-linux-gnu/glib-2.0/include \
		-I/usr/lib/x86_64-linux-gnu/dbus-1.0/include \
		-I/usr/lib/x86_64-linux-gnu

CFLAGS=-c -Wall

LINKING=`pkg-config --libs --cflags dbus-1 glib-2.0 gio-2.0 gthread-2.0 gio-unix-2.0 gobject-2.0`

all: $1

server: main.o alarm-stubs-generated.o
	$(CC) main.o alarm-stubs-generated.o -o $@ $(LINKING)

main.o: main.c alarm-stubs-generated.o
	$(CC) $(CFLAGS) main.c -o $@ $(PATHS)

alarm-stubs-generated.o: alarm-stubs-generated.c
	$(CC) $(CFLAGS) alarm-stubs-generated.c  -o $@ $(PATHS)

alarm-stubs-generated.c: alarm.xml
	gdbus-codegen --generate-c-code alarm-stubs-generated --c-namespace dbusAlarm --interface-prefix com.training.dbus. --c-generate-object-manager alarm.xml

clean:
	rm *.o server alarm-stubs-generated.c alarm-stubs-generated.h

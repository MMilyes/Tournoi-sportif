CC = gcc
CFLAGS = -Wall -g

all: OS
	doxygen config_file

projet: OS.c
	$(CC) $(CFLAGS) -o OS OS.c -lpthread

clean:
	rm -f OS

touch:
	touch Makefile

CC=gcc
CFLAGS=-Wall -Werror -g

all: shell

shell: shell.o readcmd.o

clean:
	rm -f shell shell.o readcmd.o tst tst.o

CC=gcc
CFLAGS=-Wall -Werror -g

all: tst shell

tst: tst.o readcmd.o

shell: shell.o readcmd.o

clean:
	rm -f shell shell.o readcmd.o tst tst.o

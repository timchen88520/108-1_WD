SHELL = /bin/bash
CC = gcc
CFLAGS = -g --static
SRC = $(wildcard *.c)
EXE = $(patsubst %.c, %, $(SRC))

all: ${EXE}

%:	%.c
	${CC} ${CFLAGS} $@.c -o $@ -lssh -lpthread

clean:
	rm ${EXE}


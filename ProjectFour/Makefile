CC=gcc
CFLAGS=-g -Wall -Werror
CLIBS=-lrt -lpthread

all: main.h main.c
	clear && $(CC) $(CFLAGS) main.c -o main $(CLIBS)
debug: 
	gdb main
commit:
	git add -A && git commit -m '...' && git push

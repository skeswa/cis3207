CC=gcc
CFLAGS=-g -Wall -Werror
CLIBS=-lrt -lpthread -lm

build: main.c
	rm -f main && clear && $(CC) $(CFLAGS) main.c -o main $(CLIBS)
debug: build
	gdb main
commit:
	git add -A && git commit -m '...' && git push
run: build
	./main

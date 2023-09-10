# Compiler to use for what purpose.
CC = cc
CC_DEV = clang

CFLAGS = -std=c99 $(wildcard src/*.c) -lc -lm -lcurses -Iinclude 
nyansnake : $(wildcard src/*.c) nyansnake.c
	$(CC) nyansnake.c $(CFLAGS) -o nyansnake

devbuild : $(wildcard src/*.c) naynsnake.c
	$(CC_DEV) nyansnake.c $(CFLAGS) -o nyansnake

clean :
	if [ -f nyansnake ]; then rm nyansnake; fi


CC      = gcc
CFLAGS  = -Wall -Wextra -Wpedantic -std=c99 -O2
AR      = ar
ARFLAGS = rcs

all: libsjson.a example

libsjson.a: sjson.o
	$(AR) $(ARFLAGS) $@ $^

sjson.o: sjson.c sjson.h
	$(CC) $(CFLAGS) -c -o $@ sjson.c

example: example.c libsjson.a sjson.h
	$(CC) $(CFLAGS) -o $@ example.c -L. -lsjson

clean:
	rm -f sjson.o libsjson.a example

.PHONY: all clean

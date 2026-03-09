CC      = gcc
CFLAGS  = -Wall -Wextra -Wpedantic -std=c99 -O2
AR      = ar
ARFLAGS = rcs
BUILD   = .build

all: ${BUILD}/libjsonlite.a ${BUILD}/example

${BUILD}/libjsonlite.a: ${BUILD}/jsonlite.o | ${BUILD}
	$(AR) $(ARFLAGS) $@ $^

${BUILD}/jsonlite.o: jsonlite.c jsonlite.h | ${BUILD}
	$(CC) $(CFLAGS) -c -o $@ jsonlite.c

${BUILD}/example: example.c ${BUILD}/libjsonlite.a jsonlite.h | ${BUILD}
	$(CC) $(CFLAGS) -o $@ example.c -L${BUILD} -ljsonlite

${BUILD}:
	mkdir -p ${BUILD}

clean:
	rm -f ${BUILD}

.PHONY: all clean

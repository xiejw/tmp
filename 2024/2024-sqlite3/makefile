CFLAGS         += -std=c23 -Wall -Werror -pedantic -Wfatal-errors
CFLAGS         += -Wconversion -Wshadow

CFLAGS         += $(shell pkg-config sqlite3 --cflags)
LDFLAGS        += $(shell pkg-config sqlite3 --libs)

BUIlD           = .build

.PHONY: run 01

run: 01
	${BUIlD}/01

01: 01_version.c | ${BUIlD}
	${CC} ${CFLAGS} ${LDFLAGS} -o ${BUIlD}/$@ $^

fmt:
	clang-format -i --style=file *.c

${BUIlD}:
	mkdir -p ${BUIlD}

clean:
	rm -rf ${BUIlD}

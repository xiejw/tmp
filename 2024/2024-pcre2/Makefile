CFLAGS         += -std=c23 -Wall -Werror -pedantic -Wfatal-errors
CFLAGS         += -Wconversion -Wshadow

CFLAGS         += $(shell pkg-config libpcre2-8 --cflags)
LDFLAGS        += $(shell pkg-config libpcre2-8 --libs)

BUIlD           = .build

run: compile
	${BUIlD}/demo '(?i:a)(?<inner_g>b)(c)(?!\p{N})' "Abc1Abc123Abcdabcd"

compile: | ${BUIlD}
	${CC} ${CFLAGS} ${LDFLAGS} main.c -o ${BUIlD}/demo

fmt:
	clang-format -i -style file *.c

${BUIlD}:
	mkdir -p ${BUIlD}

clean:
	rm -rf ${BUIlD}

CFLAGS         += $(shell pkg-config libpcre2-8 --cflags)
LDFLAGS        += $(shell pkg-config libpcre2-8 --libs)

BUIlD     = .build

run: compile
	${BUIlD}/demo '(?i:a)(?<inner_g>b)(c)(?!\p{N})' "Abc1Abc123Abcdabcd"

compile: | ${BUIlD}
	${CC} -std=c23 -Wall ${CFLAGS} ${LDFLAGS} main.c -o ${BUIlD}/demo

${BUIlD}:
	mkdir -p ${BUIlD}

fmt:
	clang-format -i -style file *.c

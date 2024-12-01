BUIlD     = .build

run: compile
	${BUIlD}/demo '(?i:a)bc(?!\p{N})' "Abc1Abc123Abcdabcd"

compile: | ${BUIlD}
	${CC} -std=c23 -Wall main.c -lpcre2-8 -o ${BUIlD}/demo

${BUIlD}:
	mkdir -p ${BUIlD}

fmt:
	clang-format -i -style file *.c

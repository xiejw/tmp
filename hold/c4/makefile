BUILD=.build

.PHONY: run compile

run: compile
	${BUILD}/c4

fmt:
	clang-format -i --style=file main.c

${BUILD}/c4: main.c | ${BUILD}
	cc -o $@ -g -O2 -Wall -std=c11 $<

${BUILD}:
	mkdir -p ${BUILD}

compile: ${BUILD}/c4


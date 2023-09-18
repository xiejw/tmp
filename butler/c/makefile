BUILD           = .build

CLANG_EXTS      = -iname "*.h" -o -iname "*.c"
CLANG_FMT       = clang-format -i --style=file
FMT             = sh -c 'find "$$@" ${CLANG_EXTS} | xargs ${CLANG_FMT}' sh

CMDS            = ${BUILD}/reader
OBJS            = ${BUILD}/reader.o
CFLAGS          = -std=c11 -Wall -Werror -pedantic

.PHONY: compile

#
# real stuff
#

compile: ${OBJS} ${CMDS}

${BUILD}/reader.o: reader.c | ${BUILD}
	${CC} -o $@ -c $< ${CFLAGS}

${BUILD}/reader: main.c ${OBJS} | ${BUILD}
	${CC} -o $@ $< ${CFLAGS} ${OBJS}

#
# pre-defined tasks
#

${BUILD}:
	mkdir -p $@

fmt:
	${FMT} ./

clean:
	rm -rf ${BUILD}

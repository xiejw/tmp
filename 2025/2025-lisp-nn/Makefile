BUILD    = .build

compile: ast

ast: | ${BUILD}
	${CC} ${CFLAGS} ${LDFLAGS} -o ${BUILD}/$@ $@.c

${BUILD}:
	mkdir -p ${BUILD}

fmt:
	~/Workspace/y/tools/clang_format_all.sh .

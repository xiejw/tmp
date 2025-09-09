BUILD   = .build
CFLAGS += -std=c11

run:
	mkdir -p ${BUILD} && \
		${CC} -o ${BUILD}/a.out ${CFLAGS} ${LDFLAGS} main.c && \
		${BUILD}/a.out

fmt:
	~/Workspace/y/tools/clang_format_all.sh .

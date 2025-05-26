BUILD     = .build
CFLAGS   += -std=c99 -Wall -Werror -pedantic -Wextra -Wfatal-errors -Wconversion
LDFLAGS  += -lpcre2-8

ifdef RELEASE
else
CFLAGS   += -g
endif

ifdef ASAN
CFLAGS   += -fsanitize=address
endif

run: | ${BUILD}
	clang -o  ${BUILD}/a.out ${CFLAGS} ${LDFLAGS} main.c && \
		./${BUILD}/a.out   "abc"        "abcdabce"   && \
		./${BUILD}/a.out   "abc(?!d)"   "abcdabce"   && \
		./${BUILD}/a.out   "\p{L}+"     "12测试"     && \
		echo "done"

${BUILD}:
	mkdir -p ${BUILD}

fmt:
	~/Workspace/y/tools/clang_format_all.sh .

clean:
	rm -rf ${BUILD}


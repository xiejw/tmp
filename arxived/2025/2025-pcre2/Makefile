# === Configurations -----------------------------------------------------------
#
BUILD     = .build
UNAME     = $(shell uname)

# === CFLAGS and LDFLAGS -------------------------------------------------------
#
CFLAGS   += -std=c99 -Wall -Werror -pedantic -Wextra -Wfatal-errors -Wconversion
LDFLAGS  += -lpcre2-8

ifdef RELEASE
else
CFLAGS   += -g
endif

ifdef ASAN
CFLAGS   += -fsanitize=address
endif

# Enable pcre2 from brew.
ifeq ($(UNAME), Darwin)
CFLAGS   += -I$(shell brew --prefix pcre2)/include
LDFLAGS  += -L$(shell brew --prefix pcre2)/lib
endif

# === Rules --------------------------------------------------------------------
#
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


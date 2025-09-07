CFLAGS  += -I$(shell brew --prefix libyaml)/include
LDFLAGS += -L$(shell brew --prefix libyaml)/lib -lyaml

run:
	mkdir -p .build && \
		clang -std=c11 -o .build/a.out ${CFLAGS} ${LDFLAGS} main.c && \
		.build/a.out

fmt:
	~/Workspace/y/tools/clang_format_all.sh .

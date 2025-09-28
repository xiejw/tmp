BUILD     = .build
CXXFLAGS += -std=c++17 -Wall -Werror -pedantic -Wextra -Wfatal-errors -Wconversion
LDFLAGS  += -lxml2

ifdef ASAN
CXXFLAGS += -g
LDFLAGS  += -fsanitize=address
endif

run: compile
	${BUILD}/main

compile: main

main: | ${BUILD}
	${CXX} ${CXXFLAGS} ${LDFLAGS} -o ${BUILD}/$@ $@.cc

${BUILD}:
	mkdir -p ${BUILD}

fmt:
	~/Workspace/y/tools/clang_format_all.sh .

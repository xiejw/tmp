BUILD     = .build
CXXFLAGS += -std=c++17 -Wall -Werror -pedantic -Wextra -Wfatal-errors -Wconversion

ifdef ASAN
CXXFLAGS += -g
LDFLAGS  += -fsanitize=address
endif

run: compile
	${BUILD}/ast

compile: ast

ast: | ${BUILD}
	${CXX} ${CXXFLAGS} ${LDFLAGS} -o ${BUILD}/$@ $@.cc

${BUILD}:
	mkdir -p ${BUILD}

fmt:
	~/Workspace/y/tools/clang_format_all.sh .

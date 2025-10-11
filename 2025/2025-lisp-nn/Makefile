BUILD     = .build
CXXFLAGS += -std=c++23 -Wall -Werror -pedantic -Wextra -Wfatal-errors -Wconversion

ifdef ASAN
CXXFLAGS += -g
LDFLAGS  += -fsanitize=address
endif

run: compile
	${BUILD}/ast

compile: ast

ast: | ${BUILD}
	${CXX} ${CXXFLAGS} -o ${BUILD}/main.o   -c main.cc
	${CXX} ${CXXFLAGS} -o ${BUILD}/parser.o -c parser.cc
	${CXX} ${LDFLAGS}  -o ${BUILD}/ast ${BUILD}/*.o

${BUILD}:
	mkdir -p ${BUILD}

fmt:
	~/Workspace/y/tools/clang_format_all.sh .

clean:
	rm -rf ${BUILD}

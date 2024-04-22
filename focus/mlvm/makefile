EVE_HOME  = ${HOME}/Workspace/y/ann/eve
LIBEVE    = ${EVE_HOME}/.build_release/libeve.a

CXXFLAGS += -I${EVE_HOME}/include
LDFLAGS  += ${LIBEVE}

FMT       = ~/Workspace/y/tools/clang_format_all.sh

BUILD     = .build

run: compile
	.build/main

compile: ${BUILD}/main

${BUILD}/main: cmd/main.cc ${LIBEVE} | ${BUILD}
	${CXX} -std=c++20 ${CXXFLAGS} -o .build/main ${LDFLAGS} cmd/main.cc

${LIBEVE}:
	make -C ~/Workspace/y/ann/eve release

${BUILD}:
	mkdir -p ${BUILD}

clean:
	rm -rf ${BUILD}

fmt:
	${FMT} cmd

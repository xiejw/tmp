BUILD    = .build
CXXFLAGS = -std=c++17

run: compile
	${BUILD}/main

compile: main

main: | ${BUILD}
	${CXX} ${CXXFLAGS} ${LDFLAGS} -o ${BUILD}/$@ $@.cc

${BUILD}:
	mkdir -p ${BUILD}

fmt:
	~/Workspace/y/tools/scripts/clang_format_all.sh .

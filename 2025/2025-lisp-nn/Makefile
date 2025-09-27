BUILD    = .build
CXXFLAGS = -std=c++17

compile: ast

ast: | ${BUILD}
	${CXX} ${CXXFLAGS} ${LDFLAGS} -o ${BUILD}/$@ $@.cc

${BUILD}:
	mkdir -p ${BUILD}

fmt:
	~/Workspace/y/tools/clang_format_all.sh .

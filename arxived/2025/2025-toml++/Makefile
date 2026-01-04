CXXFLAGS  += -std=c++23
CXXFLAGS  += -I${HOME}/Workspace/build/toml++/src/include

BUILD      = .build

run: ${BUILD}
	${CXX} ${CXXFLAGS} -o ${BUILD}/a.out main.cc

${BUILD}:
	mkdir -p ${BUILD}

fmt:
	~/Workspace/y/tools/clang_format_all.sh .

clean:
	rm -rf ${BUILD}

BUILD = .build

compile: | ${BUILD}
	${CXX} -std=c++23 -fprebuilt-module-path=${BUILD} -fmodule-output -c mod.ccm -o ${BUILD}/mod.o
	${CXX} -std=c++23 -fprebuilt-module-path=${BUILD} -fmodule-output -c hello.ccm -o ${BUILD}/hello.o
	${CXX} -std=c++23 -fprebuilt-module-path=${BUILD} -c hello_2.cc -o ${BUILD}/hello_2.o
	${CXX} -std=c++23 -fprebuilt-module-path=${BUILD} -c main.cc -o ${BUILD}/main.o
	${CXX} -o ${BUILD}/a.out ${BUILD}/mod.o ${BUILD}/hello.o ${BUILD}/hello_2.o ${BUILD}/main.o
	${BUILD}/a.out

${BUILD}:
	mkdir -p ${BUILD}

fmt:
	~/Workspace/y/tools/clang_format_all.sh .
clean:
	rm -rf ${BUILD}

# https://developer.apple.com/documentation/metal/1433401-mtlcreatesystemdefaultdevice
# https://zhuyie.github.io/posts/use-metal-in-command-line-app/

BUILD = .build

METAL_LIBS = ${BUILD}/add.metallib
CMD        = ${BUILD}/a.out

CFLAGS = -Wno-deprecated-declarations

LDFLAGS=-framework Foundation -framework Metal -framework MetalPerformanceShaders -framework CoreGraphics

compile: ${BUILD} ${METAL_LIBS} ${CMD}
	@echo "run with '.build/a.out'"

${CMD}: adder.m adder.h main.m
	clang++ -o ${BUILD}/adder.o -c adder.m ${CFLAGS} && \
	clang++ -o ${BUILD}/main.o -c main.m ${CFLAGS} && \
	clang++ -o ${BUILD}/a.out ${BUILD}/main.o ${BUILD}/adder.o ${LDFLAGS}

${BUILD}:
	mkdir -p ${BUILD}

${BUILD}/add.metallib: add.metal
	xcrun -sdk macosx metal -c add.metal -o ${BUILD}/add.air && \
	       xcrun -sdk macosx metallib ${BUILD}/add.air -o ${BUILD}/add.metallib

clean:
	rm -rf ${BUILD}

# https://developer.apple.com/documentation/metal/1433401-mtlcreatesystemdefaultdevice
# https://zhuyie.github.io/posts/use-metal-in-command-line-app/

BUILD = .build

METAL_LIBS = ${BUILD}/matmul.metallib
CMD        = ${BUILD}/a.out

CFLAGS = -Wno-deprecated-declarations -O2 -DNDEBUG

LDFLAGS=-framework Foundation -framework Metal -framework CoreGraphics -framework Accelerate

compile: ${BUILD} ${METAL_LIBS} ${CMD}
	@echo "run with '.build/a.out'"

run: compile
	${BUILD}/a.out

${CMD}: matmul_op.m matmul_op.h main.m const.h | ${BUILD}
	clang++ -o ${BUILD}/matmul_op.o -c matmul_op.m ${CFLAGS} && \
	clang++ -o ${BUILD}/main.o -c main.m ${CFLAGS} && \
	clang++ -o ${BUILD}/a.out ${BUILD}/main.o ${BUILD}/matmul_op.o ${LDFLAGS}

${BUILD}:
	mkdir -p ${BUILD}

${BUILD}/matmul.metallib: matmul.metal const.h | ${BUILD}
	xcrun -sdk macosx metal -c matmul.metal -o ${BUILD}/matmul.air && \
	       xcrun -sdk macosx metallib ${BUILD}/matmul.air -o ${BUILD}/matmul.metallib

clean:
	rm -rf ${BUILD}

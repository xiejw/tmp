BUILD    = .build

CFLAGS  += -O3 -march=native -std=c99 -Wall -Werror -pedantic -Wextra -Wfatal-errors -Wconversion
LDFLAGS += -lm -ffast-math

run_00: EXTRA_FLAGS = -DACCELERATE_NEW_LAPACK -framework Accelerate
run_00: | ${BUILD}
	clang -o ${BUILD}/a.out ${CFLAGS} ${LDFLAGS} ${EXTRA_FLAGS} 00_mac_blas.c && ${BUILD}/a.out

run_01: EXTRA_FLAGS = -lblas
run_01: | ${BUILD}
	clang -o ${BUILD}/a.out ${CFLAGS} ${LDFLAGS} ${EXTRA_FLAGS} 01_linux_openblas.c && ${BUILD}/a.out

run_02: EXTRA_FLAGS = -isystem ${HOME}/Workspace/build/blis/include/firestorm ${HOME}/Workspace/build/blis/lib/firestorm/libblis.a
run_02: | ${BUILD}
	clang -o ${BUILD}/a.out ${CFLAGS} ${LDFLAGS} 02_linux_blis.c ${EXTRA_FLAGS} && ${BUILD}/a.out

${BUILD}:
	mkdir -p ${BUILD}

fmt:
	~/Workspace/y/tools/clang_format_all.sh .

clean:
	rm -rf ${BUILD}

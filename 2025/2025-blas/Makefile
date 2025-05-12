BUILD    = .build

CFLAGS  += -O3 -march=native -std=c99 -Wall -Werror -pedantic -Wextra -Wfatal-errors -Wconversion
CFLAGS  += -DACCELERATE_NEW_LAPACK
LDFLAGS += -framework Accelerate -lm -ffast-math

run: | ${BUILD}
	clang -o ${BUILD}/a.out ${CFLAGS} ${LDFLAGS} 00_mac_blas.c && ${BUILD}/a.out

${BUILD}:
	mkdir -p ${BUILD}

fmt:
	~/Workspace/y/tools/clang_format_all.sh .

clean:
	rm -rf ${BUILD}

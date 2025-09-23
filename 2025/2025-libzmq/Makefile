BUILD    = .build

HOME_USR = ${HOME}/Workspace/build/usr
CFLAGS   = -I${HOME_USR}/include
LDFLAGS  = -L${HOME_USR}/lib/ -lzmq

compile: pub sub

pub: | ${BUILD}
	${CC} ${CFLAGS} ${LDFLAGS} -o ${BUILD}/$@ $@.c

sub: | ${BUILD}
	${CC} ${CFLAGS} ${LDFLAGS} -o ${BUILD}/$@ $@.c

${BUILD}:
	mkdir -p ${BUILD}

fmt:
	~/Workspace/y/tools/clang_format_all.sh .

BUILD    = .build

HOME_USR = ${HOME}/Workspace/build/usr
CFLAGS   = -I${HOME_USR}/include
LDFLAGS  = -L${HOME_USR}/lib/ -lzmq

compile: pub

pub: | ${BUILD}
	${CC} ${CFLAGS} ${LDFLAGS} -o ${BUILD}/pub pub.c

${BUILD}:
	mkdir -p ${BUILD}

BUILD   = .build

LDFLAGS = -e _start

default: ${BUILD}/hello
	${BUILD}/hello

${BUILD}/hello: ${BUILD}/hello.o
	ld ${LDFLAGS} -o ${BUILD}/hello ${BUILD}/hello.o

${BUILD}/hello.o: ${BUILD} hello.s
	as -o ${BUILD}/hello.o hello.s

${BUILD}:
	@mkdir -p ${BUILD}

clean:
	rm -rf ${BUILD}

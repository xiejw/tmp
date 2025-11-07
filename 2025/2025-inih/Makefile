BUILD  = .build

run: ${BUILD}
	${CC} -std=c99 -o ${BUILD}/ini.o  -c ini.c
	${CC} -std=c99 -o ${BUILD}/main.o -c main.c
	${CC} -o ${BUILD}/a.out ${BUILD}/*.o
	@echo "=== Example File ==="
	@cat test.ini
	@echo "=== Output       ==="
	@${BUILD}/a.out

${BUILD}:
	mkdir -p ${BUILD}

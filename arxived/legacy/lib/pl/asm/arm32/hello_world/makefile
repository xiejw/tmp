compile: bin bin/a.out

bin:
	mkdir -p bin

bin/a.out: bin/obj.o
	${CC} -o $@ $<

bin/obj.o: hello_world.s
	as -o $@ $<

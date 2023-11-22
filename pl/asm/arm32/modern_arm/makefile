CFLAGS  += -std=c99

compile: bin bin/ch02_01 bin/ch02_01.o bin/ch02_01_.o

bin/ch02_01.o: ch02_01.c
	$(CC) $(CFLAGS) -o $@ -c $<

bin/ch02_01_.o: ch02_01_.s
	$(CC) -o $@ -c $<

bin/ch02_01: bin/ch02_01_.o bin/ch02_01.o
	$(CC) -o $@ $^

bin:
	mkdir -p bin

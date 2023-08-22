# https://developer.apple.com/documentation/metal/1433401-mtlcreatesystemdefaultdevice
# https://zhuyie.github.io/posts/use-metal-in-command-line-app/

LDFLAGS=-framework Foundation -framework Metal -framework MetalPerformanceShaders -framework CoreGraphics
compile:
	xcrun -sdk macosx metal -c add.metal -o add.air && \
	xcrun -sdk macosx metallib add.air -o add.metallib && \
	clang++ -o MetalAdder.o -c MetalAdder.m && \
	clang++ -o main.o -c main.m && \
	clang++ -o a.out main.o MetalAdder.o ${LDFLAGS}

clean:
	rm -rf *.o *.air *.metallib a.out

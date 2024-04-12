run: compile
	.build/main

compile: | .build
	${CXX} -std=c++20 -o .build/main cmd/main.cc

.build:
	mkdir -p .build

clean:
	rm -rf .build

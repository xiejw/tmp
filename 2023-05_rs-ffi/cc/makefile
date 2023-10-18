cmake: .build
	Torch_DIR=~/Workspace/build/torch/install cmake -S . -B .build && \
		  cmake --build .build -v

.build:
	mkdir -p .build

clean:
	rm -rf .build

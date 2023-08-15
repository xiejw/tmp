CLANG_EXTS      = -iname '*.h' -o -iname '*.cc'
CLANG_FMT       = clang-format -i --style=file
FMT             = find "./" ${CLANG_EXTS} | xargs ${CLANG_FMT}

ane:
	clang++ -std=c++20 -Wall -O2 -DNEBUG -o demo matmul_ane.cc -framework Accelerate

torch:
	mkdir -p /tmp/build && \
		Torch_DIR=~/Workspace/build/torch/install/ cmake -S . -B /tmp/build -DCMAKE_BUILD_TYPE=Release && \
		make -C /tmp/build && \
		cp /tmp/build/torch_mps_cc demo && \
		rm -rf /tmp/build

fmt:
	${FMT}

clean:
	rm -rf demo

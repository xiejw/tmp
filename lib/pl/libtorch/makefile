# If LIBTORCH is set `LIBTORCH=1`, then it will choose the libtorch instead of
# pytorch installed version
#
ifdef LIBTORCH
LIBTORCH_PATH  = ${HOME}/Workspace/build/torch/install
CFLAGS        += -I${LIBTORCH_PATH}/include/torch/csrc/api/include/ -I${LIBTORCH_PATH}/include/
LDFLAGS       += -L${LIBTORCH_PATH}/lib
LDFLAGS       += -Wl,-rpath,${LIBTORCH_PATH}/lib
# brew install libomp
LDFLAGS       += -Wl,-rpath,/opt/homebrew/opt/libomp/lib
else
CFLAGS  += $(shell python -c 'from torch.utils import cpp_extension; print(" ".join(["-I"+x for x in cpp_extension.include_paths()]))')
LDFLAGS += $(shell python -c 'from torch.utils import cpp_extension; print(" ".join(["-L"+x for x in cpp_extension.library_paths()]))')
LDFLAGS += $(shell python -c 'from torch.utils import cpp_extension; print(" ".join(["-Wl,-rpath,"+x for x in cpp_extension.library_paths()]))')
endif

LDFLAGS += -ltorch_cpu -ltorch -lc10

run: compile
	./a.out

compile:
	clang++ -std=c++17 ${CFLAGS} ${LDFLAGS} main.cc

check:
	python -c 'import torch'

clean:
	rm a.out

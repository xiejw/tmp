CFLAGS  += $(shell python -c 'from torch.utils import cpp_extension; print(" ".join(["-I"+x for x in cpp_extension.include_paths()]))')
LDFLAGS += $(shell python -c 'from torch.utils import cpp_extension; print(" ".join(["-L"+x for x in cpp_extension.library_paths()]))')
LDFLAGS += $(shell python -c 'from torch.utils import cpp_extension; print(" ".join(["-Wl,-rpath,"+x for x in cpp_extension.library_paths()]))')
LDFLAGS += -ltorch_cpu -ltorch -lc10

compile:
	clang++ -std=c++14 ${CFLAGS} ${LDFLAGS} main.cc

check:
	python -c 'import torch'

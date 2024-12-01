# to install
#
#     pip install pybind11
compile:
	clang++ -O3 -Wall -shared -std=c++11 -fPIC -undefined dynamic_lookup \
		`python3 -m pybind11 --includes` fn.cc \
		-o fn`python3-config --extension-suffix`

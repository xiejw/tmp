# to install
#
#     pip install pybind11

compile:
	clang++ -O3 -Wall -shared -std=c++11 -fPIC -undefined dynamic_lookup \
		`python3 -m pybind11 --includes` c4.cc \
		-o xai_c4`python3-config --extension-suffix`

clean:
	rm *`python3-config --extension-suffix`


CXXFLAGS += -std=c++20 -Wall -Werror -pedantic -Wfatal-errors
CXXFLAGS += -Iinclude

# --------------------------------------------------------------------------------------------------
# fltk
#
CXXFLAGS += `fltk-config --use-images --cxxflags`
LDFLAGS  += -L`brew --prefix`/lib
LDFLAGS  += `fltk-config --use-images --ldflags`

compile:
	clang++ ${CXXFLAGS} ${LDFLAGS} main.cc
clean:
	rm a.out
fmt:
	~/Workspace/y/tools/clang_format_all.sh .

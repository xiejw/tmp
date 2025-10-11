CXXFLAGS += -std=c++17
CXXFLAGS += `pkgconf --cflags fmt`
LDFLAGS  += `pkgconf --libs fmt`
LDFLAGS  += -Wl,-w

run:
	${CXX} ${CXXFLAGS} ${LDFLAGS} -o a.out main.cc && ./a.out

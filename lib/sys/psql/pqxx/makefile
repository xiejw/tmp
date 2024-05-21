CXXFLAGS += -I${HOME}/Workspace/build/pqxx/install/include
LDFLAGS  += -L${HOME}/Workspace/build/pqxx/install/lib -lpqxx
LDFLAGS  += -L${HOME}/Workspace/build/psql/install/lib -lpq
LDFLAGS  += -Wl,-rpath ${HOME}/Workspace/build/psql/install/lib
LDFLAGS  += -Wl,-rpath ${HOME}/Workspace/build/psql/install/lib

compile:
	clang++ -Wall -std=c++20 ${CXXFLAGS}  main.cc ${LDFLAGS}


# ------------------------------------------------------------------------------
# fltk
#
FLTK_CXXFLAGS += `fltk-config --use-images --cxxflags`
FLTK_LDFLAGS  += -L`brew --prefix`/lib
FLTK_LDFLAGS  += `fltk-config --use-images --ldflags`

# ------------------------------------------------------------------------------
# general
#
BUILD          = .build
CXXFLAGS      += -std=c++20 -Wall -Werror -pedantic -Wfatal-errors
CXXFLAGS      += -Iinclude
LDFLAGS       += ${FLTK_LDFLAGS}

# ------------------------------------------------------------------------------
# app
#
BIN            = ${BUILD}/gallery
OBJS          += ${BUILD}/gallery_store.o
OBJS          += ${BUILD}/eve_fs.o

# ------------------------------------------------------------------------------
# actions
#
compile: ${BIN}

run: compile
	${BIN}

${BIN}: cmd/main.cc ${OBJS} | ${BUILD}
	clang++ ${CXXFLAGS} ${FLTK_CXXFLAGS} ${LDFLAGS} ${OBJS} -o $@ $<

${BUILD}/gallery_store.o: src/gallery/store.cc | ${BUILD}
	clang++ ${CXXFLAGS} -o $@ -c $<

${BUILD}/eve_fs.o: src/eve/fs.cc | ${BUILD}
	clang++ ${CXXFLAGS} -o $@ -c $<

${BUILD}:
	mkdir -p ${BUILD}

# ------------------------------------------------------------------------------
# housekeeping
#
clean:
	rm -rf ${BUILD}

fmt:
	~/Workspace/y/tools/clang_format_all.sh cmd include src

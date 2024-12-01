
# ------------------------------------------------------------------------------
# fltk
#
FLTK_CXXFLAGS += `fltk-config --use-images --cxxflags`
FLTK_LDFLAGS  += -L`brew --prefix`/lib
FLTK_LDFLAGS  += `fltk-config --use-images --ldflags`

# ------------------------------------------------------------------------------
# eve
#
EVE_HOME       = ${HOME}/Workspace/y/ann/eve
EVE_LIB       += ${EVE_HOME}/.build_release/libeve.a
EVE_CXXFLAGS  += -I${EVE_HOME}/include
EVE_LDFLAGS   += ${EVE_LIB}

# ------------------------------------------------------------------------------
# general
#
BUILD          = .build
CXXFLAGS      += -std=c++20 -Wall -Werror -pedantic -Wfatal-errors
CXXFLAGS      += -Iinclude
CXXFLAGS      += ${EVE_CXXFLAGS}
LDFLAGS       += ${FLTK_LDFLAGS}
LDFLAGS       += ${EVE_LDFLAGS}

# ------------------------------------------------------------------------------
# app
#
BIN            = ${BUILD}/gallery
OBJS          += ${BUILD}/gallery_app.o
OBJS          += ${BUILD}/gallery_store.o
OBJS          += ${BUILD}/eve_fs.o

# ------------------------------------------------------------------------------
# actions
#
compile: ${BIN}

run: compile
	${BIN}

${BIN}: cmd/main.cc ${OBJS} ${EVE_LIB} | ${BUILD}
	clang++ ${CXXFLAGS} ${LDFLAGS} ${OBJS} -o $@ $<

${BUILD}/gallery_app.o: src/gallery/app.cc | ${BUILD}
	clang++ ${CXXFLAGS} ${FLTK_CXXFLAGS} -o $@ -c $<

${BUILD}/gallery_store.o: src/gallery/store.cc | ${BUILD}
	clang++ ${CXXFLAGS} -o $@ -c $<

${BUILD}/eve_fs.o: src/eve/fs.cc | ${BUILD}
	clang++ ${CXXFLAGS} -o $@ -c $<

${BUILD}:
	mkdir -p ${BUILD}

${EVE_LIB}:
	make -C ${EVE_HOME} release

# ------------------------------------------------------------------------------
# housekeeping
#
clean:
	rm -rf ${BUILD}

fmt:
	~/Workspace/y/tools/clang_format_all.sh cmd include src

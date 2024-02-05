GISTS_FILES     = $(wildcard 20*.tex)
GISTS_OUTPUTS   = $(patsubst %.tex,%.pdf,${GISTS_FILES})


TEX_BIN         = docker run --rm -v `pwd`:/workdir xiejw/tex pdftex

default: ${GISTS_OUTPUTS}

%.pdf: %.tex
	${TEX_BIN} $<

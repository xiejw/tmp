NOTES_FILES     = $(wildcard *.tex)
NOTES_OUTPUTS   = $(patsubst %.tex,%.pdf,${NOTES_FILES})


TEX_BIN         = docker run --rm -v `pwd`:/workdir xiejw/tex pdftex

default: ${NOTES_OUTPUTS}

%.pdf: %.tex
	${TEX_BIN} $<

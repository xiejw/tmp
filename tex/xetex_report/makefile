BUILD=.build

.PHONY: open clean

${BUILD}/rep.pdf: rep.tex | ${BUILD}
	xelatex --output-directory=${BUILD} $<

open:
	open ${BUILD}/rep.pdf

${BUILD}:
	mkdir -p ${BUILD}

clean:
	rm -rf ${BUILD} *.log *.aux *.pdf

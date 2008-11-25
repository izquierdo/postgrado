SHELL=/bin/bash
LATEX=latex
PDFLATEX=pdflatex
BIBTEX=bibtex
DVIPDF=dvipdf

DOCUMENTO=presentacion

CAPITULOS=
EXT_BASURA={aux,log,nav,out,pdf,snm,toc,vrb}

all: ${DOCUMENTO}.pdf

${DOCUMENTO}.dvi: $(patsubst %,capitulos/%.tex,${CAPITULOS})
${DOCUMENTO}.pdf: $(patsubst %,capitulos/%.tex,${CAPITULOS})

.SUFFIXES: .tex .pdf .dvi

%.dvi: %.tex
	${LATEX} $(patsubst %.tex,%,$<)
	${BIBTEX} $(patsubst %.tex,%,$<)
	${LATEX} $(patsubst %.tex,%,$<)
	${LATEX} $(patsubst %.tex,%,$<)

.tex.pdf:
	$(PDFLATEX) $<
	$(PDFLATEX) $<

.PHONY: clean

clean:
	rm -f ${DOCUMENTO}.${EXT_BASURA} $(patsubst %,capitulos/%.${EXT_BASURA},${CAPITULOS})

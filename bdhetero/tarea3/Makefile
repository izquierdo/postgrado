SHELL=/bin/bash
LATEX=latex
PDFLATEX=pdflatex
BIBTEX=bibtex
DVIPDF=dvipdf

DOCUMENTO=tarea3

EXT_BASURA={log,aux,toc,dvi,pdf,bbl,blg,out,lot}

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

.PHONY: clean

clean:
	rm -f ${DOCUMENTO}.${EXT_BASURA}

SHELL=/bin/bash
LATEX=latex
PDFLATEX=pdflatex
BIBTEX=bibtex
DVIPDF=dvipdf

DOCUMENTO=propuesta

CAPITULOS=cronograma justificacion metodologia objetivos planteamiento portada resultados solucion trabajorelacionado
EXT_BASURA={log,aux,toc,dvi,pdf,bbl,blg,out,lot}



all: ${DOCUMENTO}.pdf

${DOCUMENTO}.pdf: $(patsubst %,secciones/%.tex,${CAPITULOS})

.SUFFIXES: .tex .pdf .dvi

.tex.pdf:
	${PDFLATEX} $<
	${BIBTEX} $(patsubst %.tex,%,$<)
	${PDFLATEX} $<
	${PDFLATEX} $<

.PHONY: clean

clean:
	rm -f ${DOCUMENTO}.${EXT_BASURA} $(patsubst %,secciones/%.${EXT_BASURA},${CAPITULOS})

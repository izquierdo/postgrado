SHELL=/bin/bash
LATEX=latex
PDFLATEX=pdflatex
BIBTEX=bibtex
DVIPDF=dvipdf

DOCUMENTO=tarea1

CAPITULOS=
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

#${BIBTEX} $(patsubst %.tex,%,$<)
#$(PDFLATEX) $<
#$(PDFLATEX) $<

.PHONY: clean

clean:
	rm -f tarea{1,2}.${EXT_BASURA} $(patsubst %,capitulos/%.${EXT_BASURA},${CAPITULOS})

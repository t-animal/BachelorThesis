.PHONY: *.pdf all clean veryclean

all: Thesis.pdf

Thesis.pdf: Thesis.tex
	latexmk -pdf -pdflatex="pdflatex -halt-on-error" -use-make $<

clean:
	latexmk -c

veryclean:
	latexmk -C


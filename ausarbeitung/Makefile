.PHONY: *.pdf all clean veryclean continous

all: Thesis.pdf

Thesis.pdf: Thesis.tex
	latexmk -pdf -pdflatex="xelatex -halt-on-error  --synctex=1" -use-make $<

continous:
	latexmk -pdf -pvc -pdflatex="xelatex -halt-on-error --synctex=1" -use-make --synctex=1Thesis.tex

clean:
	latexmk -c

veryclean:
	latexmk -C
	rm Thesis.synctex.gz


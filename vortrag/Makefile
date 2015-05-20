.PHONY: *.pdf all clean veryclean continous

all: Vortrag.pdf

Vortrag.pdf: Vortrag.tex
	latexmk -pdf -pdflatex="xelatex -halt-on-error  --synctex=1" -use-make $<

continous:
	latexmk -pdf -pvc -pdflatex="xelatex -halt-on-error --synctex=1" -use-make --synctex=1Vortrag.tex

clean:
	latexmk -c

veryclean:
	latexmk -C
	rm -f Vortrag.synctex.gz Vortrag.nav Vortrag.snm


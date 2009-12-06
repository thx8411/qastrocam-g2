SILENT = @
LATEX = pdflatex

all : fr

fr : ../manual-fr.pdf

# two passes for referencing
../manual-fr.pdf : manual-fr.tex
	$(LATEX) manual-fr.tex
	$(LATEX) manual-fr.tex
	$(SILENT) mv -f manual-??.pdf ../

clean :
	$(SILENT) rm -f ../manual-??.pdf
	$(SILENT) rm -f *.aux *.lof *.log *.toc

	

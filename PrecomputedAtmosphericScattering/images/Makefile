# set latexfile to the name of the main file without the .tex
latexfile = examplePaper
# put theassemblyWorkcellOverview1_0.eps

# if no references, run "touch latexfile.bbl; touch referencefile.bib"
# or delete dependency in .dvi and tarball sections
referencefile = refs

TEX = latex
BIBTEX = bibtex

# support subdirectories
VPATH = Figs

# reruns latex if needed.  to get rid of this capability, delete the
# three lines after the rule.  Delete .bbl dependency if not using
# BibTeX references.
# idea from http://ctan.unsw.edu.au/help/uk-tex-faq/Makefile
$(latexfile).dvi : $(figures) $(latexfile).tex
	while ($(TEX) $(latexfile) ; \
	grep -q "Rerun to get cross" $(latexfile).log ) do true ; \
	$(TEX) $(latexfile) ; \
	$(BIBTEX) $(latexfile) ; \
	$(TEX) $(latexfile) ; \
	done


# keep .eps files in the same directory as the .fig files
%.eps : %.fig
	fig2dev -L eps $< > $(dir $< )$@

$(latexfile).pdf : $(latexfile).ps myrefs.bib
	ps2pdf $(latexfile).ps $(latexfile).pdf

pdf : $(latexfile).pdf

$(latexfile).ps : $(latexfile).dvi
	dvips $(latexfile)

ps : $(latexfile).ps 

clean: 
	rm *.ps; rm *.pdf; rm *.dvi;
CURRENT=$(shell date +%Y-%m-%d)
#VERSION=1.0 1.0.1 1.1 1.2 1.3 1.3.1
#VERSION= 1.4 1.4.1 1.5 1.6 2.0
#VERSION= 2.99 2.99.2 3.0 3.1.2 3.2 3.3 3.4 3.5 3.6.1 3.6.2 3.7 3.8.1 3.9 4.0.beta.3 4.0.beta.4
#VERSION= 4.0
TARBALLS=$(VERSION:%=qastrocam-%.tar.gz) qastrocam-$(CURRENT).tar.gz

CVSROOT=:ext:fsicard@qastrocam.cvs.sourceforge.net:/cvsroot/qastrocam

include ../Makefile.vars

HOSTNAME=$(shell hostname)

ifeq ($(HOSTNAME), miniruth)
all: index translations doxygen
else
all: index translations
endif

tgz:  index $(TARBALLS)

index:: qastrocam-template.html
	@echo creating $@
	@rm -f index.html
	@sed < qastrocam-template.html > index.html s/qastrocam-CURRENT/qastrocam-$(CURRENT)/g 

lang=fr
translations:
	for i in $(lang); do lupdate ../qastrolib/*.cpp ../qastrocam/*.cpp -ts qastrocam-$$i.ts; done
	for i in $(lang); do lrelease qastrocam-$$i.ts -qm qastrocam-$$i.qm; done

qastrocam-%.tar.gz:%
	@(test -d qastrocam-$< && ( echo cleaning qastrocam-$<; rm -fr qastrocam-$< ) ) || true
	@echo extracting version $<
	cvs -Q -d $(CVSROOT) export -r R_`echo $< | sed "s/\./_/g"` -d qastrocam-$< qastrocam
	@$(MAKE) -C qastrocam-$< configure-debian-mt
	@echo building doc
	@$(MAKE) -C qastrocam-$</qastrolib doc
	@echo building source tarball
	@make clean -C qastrocam-$<
	@tar -czf $@  --exclude alignlib qastrocam-$<
	@echo building binary
	@$(MAKE) -C qastrocam-$< configure-debian-mt
	@$(MAKE) -C qastrocam-$< package STATIC_MODE=false OPTFLAGS="-O3" > /dev/null
	@echo building binary tarball
	@cp qastrocam-$</qastrocam-$<.tgz qastrocam-$<-binary.tar.gz
	@echo cleaning
	@rm -fr qastrocam-$<

qastrocam-$(CURRENT).tar.gz:
	echo extracting version $(CURRENT)
	rm -fr qastrocam-200?-??-??*
	cvs -Q -d $(CVSROOT) export -d qastrocam-$(CURRENT) -r HEAD qastrocam
	@$(MAKE) -C qastrocam-$(CURRENT) configure-debian-mt
	@echo building doc
	@$(MAKE) -C qastrocam-$(CURRENT)/qastrolib doc
	@echo building source tarball
	@make clean -C qastrocam-$(CURRENT)
	@tar -czf $@  --exclude alignlib qastrocam-$(CURRENT)
	@echo building binary tarball
	@$(MAKE) -C qastrocam-$(CURRENT) configure-debian-mt
	@$(MAKE) -C qastrocam-$(CURRENT) package STATIC_MODE=false OPTFLAGS="-O3" > /dev/null
	@cp qastrocam-$(CURRENT)/qastrocam-$(CURRENT).tgz qastrocam-$(CURRENT)-binary.tar.gz
	@echo cleaning
	@rm -fr qastrocam-$(CURRENT)

clean::
	rm -f qastrocam-[0-9]*-[0-9]*-[0-9].tar.gz qastrocam-[0-9]*-[0-9]*-[0-9]-binary.tar.gz index.html *.qm
distclean:: clean
	rm -f $(VERSION) $(TARBALLS) qastrocam-*.bin.gz index.html
$(VERSION):
	@touch $@
doxygen:
	$(MAKE) -C ../qastrolib doc
install::all
	install -d $(DESTDIR)/share/doc/qastrocam/
	rsync -a --cvs-exclude --exclude "*.qm" . $(DESTDIR)/share/doc/qastrocam/.
	install -d $(DESTDIR)/share/qastrocam/locales
	install *.qm $(DESTDIR)/share/qastrocam/locales/.

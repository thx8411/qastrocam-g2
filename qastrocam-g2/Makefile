SHELL=/bin/bash
SUBDIRS=qastrolib qastrocam doc
#SUBDIRS+=alignlib qastrostack qastroscope

-include Makefile.vars

all:: configure_done
	for i in $(SUBDIRS) ; do $(MAKE) $(FAST_FLAGS) -C $$i || exit 1; done
full:: all

install:: all 
	for i in $(SUBDIRS); do $(MAKE) -C $$i install TARBALLS= ; done
	iconsDir="$(DESTDIR)/share/qastrocam/icons"; install -d "$$iconsDir"; install icons/*.png "$$iconsDir"

release::
	$(MAKE) all OPTFLAGS="-O3"
clean::configure_done
	$(MAKE) -C qastrolib clean
	$(MAKE) -C qastrocam clean
distclean::
	(test -f configure_done && $(MAKE) clean ) || true
	rm -f configure_done Makefile.vars
	rm -f config.h config.log
	rm -f configure config.cache config.h.in

depend dep::configure_done
	$(MAKE) -C qastrolib depend
	$(MAKE) -C qastrocam depend
package::release
	#fakeroot ./debian/rules binary
	strip bin/* || true
	#ln -sf `ldd bin/qastrocam  | egrep "libqt|libSDL" | sed "s/[ 	][ 	]*/ /g" | cut -f 4 -d" "` bin
	dir=`basename $$PWD`;cd ..; rm -f $$dir/$$dir.tgz;tar --exclude CVS -chvzf $$dir/$$dir.tgz $$dir/{icons,bin,doc} 
commit::
	cvs update
	vi qastrocam/qastrocamVersion.hpp
	$(MAKE) all
	cvs commit

configure_done: Makefile.vars.in configure config.h.in configure.in
	@$(MAKE) show-avaible-arch

configure: configure.in config.h.in
	rm -fr autom4te.cache
	autoconf
	rm -fr autom4te.cache
config.h.in: configure.in
	rm -f config.h.in
	autoheader
deb::
	dpkg-buildpackage -rfakeroot

include Makefile.arch

# DO NOT DELETE

# Qastrocam
# Copyright (C) 2003-2009   Franck Sicard
# Qastrocam-g2
# Copyright (C) 2009-2010   Blaise-Florentin Collin

# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License v2
# as published by the Free Software Foundation.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
# MA  02110-1301, USA.

SHELL=/bin/bash
SUBDIRS=qastrolib qastrocam

-include Makefile.vars

all:: configure_done
	for i in $(SUBDIRS) ; do $(MAKE) $(FAST_FLAGS) -C $$i || exit 1; done
full:: all

install:: all 
	for i in $(SUBDIRS); do $(MAKE) -C $$i install TARBALLS= ; done
	iconsDir="$(DESTDIR)/share/qastrocam-g2/icons"; install -d "$$iconsDir"; install icons/*.png "$$iconsDir"
	gnomeMenuDir="$(DESTDIR)/share/applications"; install -d "$$gnomeMenuDir"; install qastrocam-g2.desktop "$$gnomeMenuDir"
	gnomeIconDir="$(DESTDIR)/share/icons"; install -d "$$gnomeIconDir"; install icons/qastrocam-g2-icon.png "$$gnomeIconDir"
	dllDir="$(DESTDIR)/lib/win32"; install -d "$$dllDir"; install lib/huffyuv.dll "$$dllDir"
	rm -f /tmp/qastrocam-g2_shift.fifo
	fifoDir="/tmp"; install -d "$$fifoDir"; mkfifo /tmp/qastrocam-g2_shift.fifo

release::
	$(MAKE) all
clean::configure_done
	$(MAKE) -C qastrolib clean
	$(MAKE) -C qastrocam clean
	$(MAKE) -C doc/sources clean
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

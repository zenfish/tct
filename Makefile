SHELL	= /bin/sh
CC	= gcc
DIRS	= bin etc

all:	$(DIRS) compile

$(DIRS):
	mkdir -p $@

re-config: reconfig
	@test -x bin/strip_tct_home || perl reconfig

compile: re-config
	chmod 700 .
	cd src/aux; make "CC=$(CC)" MAKELEVEL=
	cd src/misc; make "CC=$(CC)" MAKELEVEL=
	cd src/fstools; make "CC=$(CC)" MAKELEVEL=
	cd src/pcat; make "CC=$(CC)" MAKELEVEL=
	cd src/file; make "CC=$(CC)" MAKELEVEL=
	cd src/lastcomm; make "CC=$(CC)" MAKELEVEL=
	cd src/major_minor; make "CC=$(CC)" MAKELEVEL=
	cd extras/entropy; make "CC=$(CC)" MAKELEVEL=
	cd extras/findkey; make "CC=$(CC)" MAKELEVEL=
	bin/major_minor > lib/major_minor.pl
	rm -f bin/lazarus
	ln -s `pwd`/lazarus/lazarus bin/lazarus

manpages depend:
	cd src/aux; make $@ "CC=$(CC)" MAKELEVEL=
	cd src/misc; make $@ "CC=$(CC)" MAKELEVEL=
	cd src/fstools; make $@ "CC=$(CC)" MAKELEVEL=
	cd src/pcat; make $@ "CC=$(CC)" MAKELEVEL=
	cd src/file; make $@ "CC=$(CC)" MAKELEVEL=
	cd src/lastcomm; make $@ "CC=$(CC)" MAKELEVEL=
	cd src/major_minor; make $@ "CC=$(CC)" MAKELEVEL=
	cd extras/entropy; make $@ "CC=$(CC)" MAKELEVEL=
	cd extras/findkey; make $@ "CC=$(CC)" MAKELEVEL=

clean: re-config
	rm -f bin/lazarus
	rm -f bin/major_minor
	rm -f lib/major_minor.pl
	cd src/aux; make $@ "CC=$(CC)" MAKELEVEL=
	cd src/misc; make $@ "CC=$(CC)" MAKELEVEL=
	cd src/fstools; make $@ "CC=$(CC)" MAKELEVEL=
	cd src/pcat; make $@ "CC=$(CC)" MAKELEVEL=
	cd src/file; make $@ "CC=$(CC)" MAKELEVEL=
	cd src/lastcomm; make $@ "CC=$(CC)" MAKELEVEL=
	cd src/major_minor; make $@ "CC=$(CC)" MAKELEVEL=
	cd extras/entropy; make $@ "CC=$(CC)" MAKELEVEL=
	cd extras/findkey; make $@ "CC=$(CC)" MAKELEVEL=

tidy: clean
	rm -f error-log *core *.old *.bak *.orig .nfs* .pure *- make.* *.tmp* \
	*/*/*.old */*/*.bak */*/*.orig */*/.nfs* */*/.pure */*/*- */*/make.* \
	*/*.old */*.bak */*.orig */.nfs* */.pure */*- */make.* typescript
	rm -rf data
	rm -f bin/lazarus
	rm -f coroner.log
	rm -f error.log
	rm -f bin/major_minor
	rm -f lib/major_minor.pl
	bin/strip_tct_home
	find . -type f -print | xargs chmod 644
	find . -type d -print | xargs chmod 755


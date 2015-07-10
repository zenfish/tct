SHELL	= /bin/sh
OPT	= -O
DEBUG	= -g
INCL	= -I../aux
CFLAGS	= $(INCL) $(DEFS) $(OPT) $(DEBUG)
LIBS	= ../aux/aux_lib.a
BIN_DIR	= ../../bin
MAN_DIR	= ../../man
PROGS	= $(BIN_DIR)/pcat
MANPAGES= $(MAN_DIR)/man1/pcat.1
AR	= ar rv
RANLIB	= ranlib

what:
	@sh makedefs

all:	$(PROGS)

manpages: $(MANPAGES)

$(BIN_DIR)/pcat: pcat.o $(LIBS)
	$(CC) $(CFLAGS) -o $@ pcat.o $(LIBS) $(SYSLIBS)

$(MAN_DIR)/man1/pcat.1: pcat.c
	srctoman $? >$@

clean:
	rm -f $(PROGS) *.o *core

depend: $(MAKES)
	(sed '1,/^# do not edit/!d' Makefile; \
	set -e; for i in [a-z][a-z0-9]*.c; do \
	    $(CC) -E $(DEFS) $(INCL) $$i | sed -n -e '/^# *1 *"\([^"]*\)".*/{' \
	    -e 's//'`echo $$i|sed 's/c$$/o/'`': \1/' -e 'p' -e '}'; \
	done) | grep -v '[.][o][:][ ][/]' >$$$$ && mv $$$$ Makefile

# do not edit - this was generated with make depend
pcat.o: pcat.c
pcat.o: ../aux/error.h
pcat.o: ../aux/mymalloc.h

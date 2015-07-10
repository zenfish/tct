SHELL	= /bin/sh
CC	= gcc
OPT	= -O
DEBUG	= -g
CFLAGS	= $(DEFS) $(OPT) $(DEBUG)
LIBOBJ	= error.o strerror.o 
BIN_DIR	= ../../bin
MAN_DIR	= ../../man
PROGS	= $(BIN_DIR)/major_minor
MANPAGES= $(MAN_DIR)/man1/major_minor.1
AR	= ar rv
RANLIB	= ranlib

what:
	@sh makedefs

all:	$(PROGS)

manpages: $(MANPAGES)

$(BIN_DIR)/major_minor: major_minor.o
	$(CC) $(CFLAGS) -o $@ major_minor.o $(SYSLIBS)

$(MAN_DIR)/man1/major_minor.1:major_minor.c
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
major_minor.o: major_minor.c

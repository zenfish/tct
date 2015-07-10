SHELL	= /bin/sh
CC	= gcc
OPT	= -O
DEBUG	= -g
INCL	= -I.
CFLAGS	= $(DEFS) $(INCL) $(OPT) $(DEBUG)
OBJS	= lastcomm.o $(MISSING_OBJS)
BIN_DIR	= ../../bin
MAN_DIR	= ../../man
PROGS	= $(BIN_DIR)/lastcomm
MANPAGES= $(MAN_DIR)/man1/lastcomm.1
MAKES	= lastcomm.c lastcomm.1

what:
	@sh makedefs

all:	$(PROGS)

manpages: $(MANPAGES)

$(BIN_DIR)/lastcomm: $(OBJS)
	$(CC) -o $@ $(OBJS) $(SYSLIBS)

$(MAN_DIR)/man1/lastcomm.1: lastcomm.1
	rm -f $@
	ln $? $@
clean:
	rm -f $(PROGS) *.o *core

#lastcomm.c: lastcomm.c.diff lastcomm.c.freebsd
#	cp lastcomm.c.freebsd lastcomm.c
#	patch lastcomm.c <lastcomm.c.diff

#lastcomm.1: lastcomm.1.diff lastcomm.1.freebsd
#	cp lastcomm.1.freebsd lastcomm.1
#	patch lastcomm.1 <lastcomm.1.diff

diffs:	lastcomm.c.freebsd lastcomm.c lastcomm.1.freebsd lastcomm.1
	-diff -c lastcomm.c.freebsd lastcomm.c >lastcomm.c.diff
	-diff -c lastcomm.1.freebsd lastcomm.1 >lastcomm.1.diff

depend: $(MAKES)
	(sed '1,/^# do not edit/!d' Makefile; \
	set -e; for i in [a-z][a-z0-9]*.c; do \
	    $(CC) -E $(DEFS) $(INCL) $$i | sed -n -e '/^# *1 *"\([^"]*\)".*/{' \
	    -e 's//'`echo $$i|sed 's/c$$/o/'`': \1/' -e 'p' -e '}'; \
	done) | grep -v '[.][o][:][ ][/]' >$$$$ && mv $$$$ Makefile

# do not edit - this was generated with make depend
lastcomm.o: lastcomm.c
lastcomm.o: sys_defs.h

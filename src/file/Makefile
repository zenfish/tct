# Copyright (c) Ian F. Darwin 86/09/01 - see LEGAL.NOTICE.
# @(#)$Id: Makefile,v 1.5.2.1 1996/12/14 10:10:42 joerg Exp $
#
# This software is not subject to any license of the American Telephone
# and Telegraph Company or of the Regents of the University of California.
#
# Permission is granted to anyone to use this software for any purpose on
# any computer system, and to alter it and redistribute it freely, subject
# to the following restrictions:
#
# 1. The author is not responsible for the consequences of use of this
#    software, no matter how awful, even if they arise from flaws in it.
#
# 2. The origin of this software must not be misrepresented, either by
#    explicit claim or by omission.  Since few users ever read sources,
#    credits must appear in the documentation.
#
# 3. Altered versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.  Since few users
#    ever read sources, credits must appear in the documentation.
#
# 4. This notice may not be removed or altered.

SHELL	= /bin/sh
CC	= gcc
OPT	= -O
DEBUG	= -g
CFLAGS	= $(OPT) $(DEBUG) $(DEFS)
DEFS	= -DMAGIC='"'"`cd $(CONF); pwd`"'/magic"'
OBJS	= file.o apprentice.o fsmagic.o softmagic.o ascmagic.o \
	compress.o is_tar.o  print.o international.o
BIN	= ../../bin
CONF	= ../../etc
MAN	= ../../man
PROG	= $(BIN)/file
MAGIC	= $(CONF)/magic

MAN1	= $(MAN)/man1/file.1
MAN5	= $(MAN)/man5/magic.5

MAGFILES= Magdir/Header Magdir/Localstuff Magdir/[a-z]*

what:
	@sh makedefs

all:	$(PROG) $(MAGIC)

manpages: $(MAN1) $(MAN5)

$(MAGIC): $(MAGFILES)
	cat $(MAGFILES) > $@

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

$(MAN)/man1/file.1: file.1
	rm -f $@
	ln $? $@

$(MAN)/man5/magic.5: magic.5
	rm -f $@
	ln $? $@

clean:
	rm -f $(PROG) file $(MAGIC) magic *.o *core *.out

depend: $(MAKES)
	(sed '1,/^# do not edit/!d' Makefile; \
	set -e; for i in [a-z][a-z0-9]*.c; do \
	    $(CC) -E $(DEFS) $(INCL) $$i | sed -n -e '/^# *1 *"\([^"]*\)".*/{' \
	    -e 's//'`echo $$i|sed 's/c$$/o/'`': \1/' -e 'p' -e '}'; \
	done) | grep -v '[.][o][:][ ][/]' >$$$$ && mv $$$$ Makefile

# do not edit below this line - it is generated by 'make depend'
apprentice.o: apprentice.c
apprentice.o: file.h
ascmagic.o: ascmagic.c
ascmagic.o: file.h
ascmagic.o: names.h
compress.o: compress.c
compress.o: file.h
file.o: file.c
file.o: patchlevel.h
file.o: file.h
fsmagic.o: fsmagic.c
fsmagic.o: file.h
international.o: international.c
international.o: international.c
international.o: file.h
is_tar.o: is_tar.c
is_tar.o: tar.h
print.o: print.c
print.o: file.h
softmagic.o: softmagic.c
softmagic.o: file.h

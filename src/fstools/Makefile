SHELL	= /bin/sh
CC	= gcc
OPT	= -O
DEBUG	= -g
INCL	= -I../aux
CFLAGS	= $(DEFS) $(INCL) $(OPT) $(DEBUG)
LIBOBJ	= fs_buf.o fs_inode.o fs_io.o fs_copy_file.o fs_open.o ffs.o \
	ffs2.o ext2fs.o mylseek.o
LIBS	= ../aux/aux_lib.a
LIB	= fs_lib.a
BIN_DIR	= ../../bin
MAN_DIR	= ../../man
PROGS	= $(BIN_DIR)/ils $(BIN_DIR)/unrm $(BIN_DIR)/icat
MANPAGES= $(MAN_DIR)/man1/icat.1 $(MAN_DIR)/man1/ils.1 $(MAN_DIR)/man1/unrm.1

what:
	@sh makedefs

all:	$(LIB) $(PROGS)

manpages: $(MANPAGES)

$(LIB):	$(LIBOBJ)
	$(AR) $@ $?
	$(RANLIB) $@

$(BIN_DIR)/unrm: unrm.o $(LIB) $(LIBS)
	$(CC) $(CFLAGS) -o $@ unrm.o $(LIB) $(LIBS) $(SYSLIBS)

$(BIN_DIR)/ils: ils.o $(LIB) $(LIBS)
	$(CC) $(CFLAGS) -o $@ ils.o $(LIB) $(LIBS) $(SYSLIBS)

$(BIN_DIR)/icat: icat.o $(LIB) $(LIBS)
	$(CC) $(CFLAGS) -o $@ icat.o $(LIB) $(LIBS) $(SYSLIBS)

$(MAN_DIR)/man1/icat.1: icat.c
	srctoman $? >$@

$(MAN_DIR)/man1/ils.1: ils.c
	srctoman $? >$@

$(MAN_DIR)/man1/unrm.1: unrm.c
	srctoman $? >$@

clean:
	rm -f $(PROGS) $(LIB) *.o *core

depend: $(MAKES)
	(sed '1,/^# do not edit/!d' Makefile; \
	set -e; for i in [a-z][a-z0-9]*.c; do \
	    $(CC) -E $(DEFS) $(INCL) $$i | sed -n -e '/^# *1 *"\([^"]*\)".*/{' \
	    -e 's//'`echo $$i|sed 's/c$$/o/'`': \1/' -e 'p' -e '}'; \
	done) | grep -v '[.][o][:][ ][/]' >$$$$ && mv $$$$ Makefile

# do not edit - this was generated with make depend
ext2fs.o: ext2fs.c
ext2fs.o: fs_tools.h
ffs.o: ffs.c
ffs.o: fs_tools.h
ffs2.o: ffs2.c
ffs2.o: fs_tools.h
fs_buf.o: fs_buf.c
fs_buf.o: fs_tools.h
fs_buf.o: ../aux/mymalloc.h
fs_buf.o: ../aux/error.h
fs_copy_file.o: fs_copy_file.c
fs_copy_file.o: fs_tools.h
fs_copy_file.o: ../aux/mymalloc.h
fs_copy_file.o: ../aux/error.h
fs_inode.o: fs_inode.c
fs_inode.o: fs_tools.h
fs_inode.o: ../aux/mymalloc.h
fs_inode.o: ../aux/error.h
fs_io.o: fs_io.c
fs_io.o: fs_tools.h
fs_io.o: ../aux/mymalloc.h
fs_io.o: ../aux/error.h
fs_open.o: fs_open.c
fs_open.o: fs_tools.h
fs_open.o: ../aux/error.h
icat.o: icat.c
icat.o: fs_tools.h
icat.o: ../aux/error.h
ils.o: ils.c
ils.o: fs_tools.h
ils.o: ../aux/error.h
ils.o: ../aux/split_at.h
mylseek.o: mylseek.c
mylseek.o: fs_tools.h
unrm.o: unrm.c
unrm.o: fs_tools.h
unrm.o: ../aux/error.h
unrm.o: ../aux/split_at.h

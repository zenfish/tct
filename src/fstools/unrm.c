/*++
/* NAME
/*	unrm 1
/* SUMMARY
/*	disk data recovery
/* SYNOPSIS
/* .ad
/* .fi
/*	\fBunrm\fR [\fB-bevV\fR] [\fB-f \fIfstype\fR]
/*		\fIdevice\fR [\fIstart-stop\fR ...]
/* DESCRIPTION
/*	\fBunrm\fR opens the named \fIdevice\fR and copies data blocks.
/*	By default, \fBunrm\fR copies unallocated data blocks only.
/*
/* 	Arguments:
/* .IP \fB-b\fR
/*	With file systems that have logical blocks that consist of fragments,
/*	don't insert null-byte padding to preserve logical block alignment
/*	in the output.
/*	This option is a no-op with the LINUX ext2fs file system, where
/*	logical blocks and fragments have the same size.
/* .IP \fB-e\fR
/*	Copy every block. The output should be similar to dd(1).
/* .IP "\fB-f\fI fstype\fR"
/*	Specifies the file system type. The default file system type
/*	is system dependent. With most UNIX systems the default type
/*	is \fBufs\fR (Berkeley fast file system). With Linux the default
/*	type is \fBext2fs\fR (second extended file system).
/* .IP \fB-v\fR
/*	Turn on verbose mode, output to stderr.
/* .IP \fB-V\fR
/*	Turn on verbose mode, output to stdout.
/* .IP \fIdevice\fR
/*	Disk special file, or regular file containing a disk image.
/*	On UNIX systems, raw mode disk access may give better performance
/*	than block mode disk access.  LINUX disk device drivers support
/*	only block mode disk access.
/* .IP "\fIstart-stop\fR ..."
/*	Examine the specified block number or number range. Either the
/*	\fIstart\fR, the \fIstop\fR, or the \fI-stop\fR may be omitted.
/*	If \fB-b\fR is not specified, the start block must be
/*	aligned to a logical block boundary (e.g. a multiple of 8 in
/*	the case of an FFS file system). With the LINUX ext2fs file system,
/*	the start block number must be >= 1.
/* BUGS
/*	\fBunrm\fR should support more file system types. Right now, support
/*	is limited to \fBext2fs\fR when built on Linux, and \fBufs\fR when
/*	built on Solaris and BSD systems.
/* LICENSE
/*	This software is distributed under the IBM Public License.
/* AUTHOR(S)
/*	Wietse Venema
/*	IBM T.J. Watson Research
/*	P.O. Box 704
/*	Yorktown Heights, NY 10598, USA
/*--*/

#include "fs_tools.h"
#include "error.h"
#include "split_at.h"

FILE   *logfp;

/* atoblock - convert string to block number */

DADDR_T atoblock(const char *str)
{
    char   *cp;
    DADDR_T addr;

    if (*str == 0)
	return (0);
    addr = STRTOUL(str, &cp, 0);
    if (*cp || cp == str)
	error("bad block number: %s", str);
    return (addr);
}

/* usage - explain and terminate */

static void usage()
{
    error("usage: %s [-b (no block padding)] [-e (every block)] [-f fstype] [-vV] device [block... ]",
	  progname);
}

/* print_block - write data block to stdout */

static void print_block(DADDR_T addr, char *buf, int flags, char *ptr)
{
    FS_INFO *fs = (FS_INFO *) ptr;

    if (verbose)
	fprintf(logfp, "write block %lu\n", (ULONG) addr);
    if (fwrite(buf, fs->block_size, 1, stdout) != 1)
	error("write stdout: %m");
}

/* main - open file system, list block info */

int     main(int argc, char **argv)
{
    FS_INFO *fs;
    char   *start;
    char   *last;
    DADDR_T bstart;
    DADDR_T blast;
    int     ch;
    int     flags = FS_FLAG_UNALLOC | FS_FLAG_ALIGN;
    char   *fstype = DEF_FSTYPE;

    progname = argv[0];

    while ((ch = getopt(argc, argv, "bef:vV")) > 0) {
	switch (ch) {
	default:
	    usage();
	case 'b':
	    flags &= ~FS_FLAG_ALIGN;
	    break;
	case 'e':
	    flags |= FS_FLAG_ALLOC;
	    break;
	case 'f':
	    fstype = optarg;
	    break;
	case 'v':
	    verbose++;
	    logfp = stderr;
	    break;
	case 'V':
	    verbose++;
	    logfp = stdout;
	    break;
	}
    }

    if (optind >= argc)
	usage();

    /*
     * Open the file system.
     */
    fs = fs_open(argv[optind++], fstype);

    /*
     * Output the named data blocks, subject to the specified restrictions.
     */
    if (optind < argc) {
	while ((start = argv[optind]) != 0) {
	    last = split_at(start, '-');
	    bstart = (*start ? atoblock(start) : fs->start_block);
	    blast = (!last ? bstart : *last ? atoblock(last) : fs->last_block);
	    fs->block_walk(fs, bstart, blast, flags, print_block, (char *) fs);
	    optind++;
	}
    }

    /*
     * Output all blocks, subject to the specified restrictions.
     */
    else {
	fs->block_walk(fs, fs->start_block, fs->last_block,
		       flags, print_block, (char *) fs);
    }
    fs->close(fs);
    exit(0);
}

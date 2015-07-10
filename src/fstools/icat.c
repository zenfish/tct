/*++
/* NAME
/*	icat 1
/* SUMMARY
/*	copy files by inode number
/* SYNOPSIS
/* .ad
/* .fi
/*	\fBicat\fR [\fB-hHvV\fR] [\fB-f \fIfstype\fR]
/*		\fIdevice\fR \fIinode\fR ...
/* DESCRIPTION
/*	\fBicat\fR opens the named \fIdevice\fR and copies the files
/*	with the specified \fIinode\fR numbers to standard output.
/*
/*	 Arguments:
/* .IP "\fB-f \fIfstype\fR"
/*	Specifies the file system type. The default file system type
/*	is system dependent. With most UNIX systems the default type
/*	is \fBufs\fR (Berkeley fast file system). With Linux the default
/*	type is \fBext2fs\fR (second extended file system).
/* .IP \fB-h\fR
/*	Skip over holes in files, so that absolute address information
/*	is lost. This option saves space when copying sparse files.
/* .IP "\fB-H\fR (default)"
/*	Copy holes in files as null blocks, so that absolute address
/*	information is preserved. This option wastes space when copying
/*	sparse files.
/* .IP \fB-v\fR
/*	Enable verbose mode, output to stderr.
/* .IP \fB-V\fR
/*	Enable verbose mode, output to stdout.
/* .IP \fIdevice\fR
/*	Disk special file, or regular file containing a disk image.
/*	On UNIX systems, raw mode disk access may give better performance
/*	than block mode disk access.  LINUX disk device drivers support
/*	only block mode disk access.
/* .IP \fIinode\fR
/*	Inode number. \fBicat\fR concatenates the contents of all specified
/*	files.
/* BUGS
/*	\fBicat\fR should support more file system types. Right now, support
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

FILE   *logfp;

/* usage - explain and terminate */

static void usage()
{
    error("usage: %s [-f fstype] [-h (no holes)] [-H (keep holes)] [-vV] device inum...", progname);
}

int     main(int argc, char **argv)
{
    FS_INFO *fs;
    char   *cp;
    INUM_T  inum;
    int     flags = FS_FLAG_ALLOC | FS_FLAG_UNALLOC;
    int     ch;
    char   *fstype = DEF_FSTYPE;

    progname = argv[0];

    while ((ch = getopt(argc, argv, "f:hHvV")) > 0) {
	switch (ch) {
	default:
	    usage();
	case 'f':
	    fstype = optarg;
	    break;
	case 'h':
	    flags &= ~FS_FLAG_UNALLOC;
	    break;
	case 'H':
	    flags |= FS_FLAG_UNALLOC;
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

    if (argc < optind + 2)
	usage();

    fs = fs_open(argv[optind], fstype);

    while (argv[++optind]) {
	inum = STRTOUL(argv[optind], &cp, 0);
	if (*cp || cp == argv[optind])
	    usage();
	fs_copy_file(fs, inum, flags);
    }
    fs->close(fs);
    exit(0);
}

/*++
/* NAME
/*	ils 1
/* SUMMARY
/*	list inode information
/* SYNOPSIS
/* .ad
/* .fi
/*	\fBils\fR [\fB-eorvV\fR] [\fB-f \fIfstype\fR]
/*		\fIdevice\fR [\fIstart-stop\fR ...]
/*
/*	\fBils\fR [\fB-aAlLvVzZ\fR] [\fB-f \fIfstype\fR]
/*		\fIdevice\fR [\fIstart-stop\fR ...]
/* DESCRIPTION
/*	\fBils\fR opens the named \fIdevice\fR and lists inode information.
/*	By default, \fBils\fR lists only the inodes of removed files.
/*
/* 	Arguments:
/* .IP \fB-e\fR
/*	List every inode in the file system.
/* .IP "\fB-f\fI fstype\fR"
/*	Specifies the file system type. The default file system type
/*	is system dependent. With most UNIX systems the default type
/*	is \fBffs\fR (Berkeley fast file system). With Linux the default
/*	type is \fBext2fs\fR (second extended file system).
/* .IP \fB-o\fR
/*	List only inodes of removed files that are still open or executing.
/*	This option is short-hand notation for \fB-aL\fR
/*	(see the \fBfine controls\fR section below).
/* .IP \fB-r\fR
/*	List only inodes of removed files. This option is short-hand notation
/*	for \fB-LZ\fR
/*	(see the \fBfine controls\fR section below).
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
/*	Examine the specified inode number or number range. Either the
/*	\fIstart\fR, the \fIstop\fR, or the \fI-stop\fR may be omitted.
/* .PP
/*	Fine controls:
/* .IP \fB-a\fR
/*	List only allocated inodes: these belong to files with at least one
/*	directory entry in the file system, and to removed files that
/*	are still open or executing.
/* .IP \fB-A\fR
/*	List only unallocated inodes: these belong to files that no longer
/*	exist.
/* .IP \fB-l\fR
/*	List only inodes with at least one hard link. These belong to files
/*	with at least one directory entry in the file system.
/* .IP \fB-L\fR
/*	List only inodes without any hard links. These belong to files that no
/*	longer exist, and to removed files that are still open or executing.
/* .IP \fB-z\fR
/*	List only inodes with zero status change time. Presumably, these
/*	inodes were never used.
/* .IP \fB-Z\fR
/*	List only inodes with non-zero status change time. Presumably, these
/*	belong to files that still exist, or that existed in the past.
/* .PP
/*	The output format is in time machine format, as described in
/*	tm-format(5). The output begins with a two-line header that
/*	describes the data origin, and is followed by a one-line header
/*	that lists the names of the data attributes that make up the
/*	remainder of the output:
/* .IP \fBst_ino\fR
/*	The inode number.
/* .IP \fBst_alloc\fR
/*	Allocation status: `a' for allocated inode, `f' for free inode.
/* .IP \fBst_uid\fR
/*	Owner user ID.
/* .IP \fBst_gid\fR
/*	Owner group ID.
/* .IP \fBst_mtime\fR
/*	UNIX time (seconds) of last file modification.
/* .IP \fBst_atime\fR
/*	UNIX time (seconds) of last file access.
/* .IP \fBst_ctime\fR
/*	UNIX time (seconds) of last inode status change.
/* .IP \fBst_dtime\fR
/*	UNIX time (seconds) of file deletion (LINUX only).
/* .IP \fBst_mode\fR
/*	File type and permissions (octal).
/* .IP \fBst_nlink\fR
/*	Number of hard links.
/* .IP \fBst_size\fR
/*	File size in bytes.
/* .IP \fBst_block0,st_block1\fR
/*	The first two entries in the direct block address list.
/* SEE ALSO
/*	mactime(1), mtime, atime, ctime reporter
/*	tm-format(5), time machine data format
/* BUGS
/*	\fBils\fR should support more file system types. Right now, support
/*	is limited to \fBext2fs\fR when built on Linux, and \fBffs\fR when
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

#define DEF_FLAGS	(FS_FLAG_USED | FS_FLAG_UNLINK)

/* atoinum - convert string to inode number */

INUM_T  atoinum(const char *str)
{
    char   *cp;
    INUM_T  inum;

    if (*str == 0)
	return (0);
    inum = STRTOUL(str, &cp, 0);
    if (*cp || cp == str)
	error("bad inode number: %s", str);
    return (inum);
}

/* usage - explain and terminate */

static void usage()
{
    error("usage: %s [-e (everything)] [-f fstype] [-o (removed but still open)] [-r (removed)] [-vV] device [inum... ]",
	  progname);
}

/* print_header - print time machine header */

static void print_header(const char *device)
{
    char    hostnamebuf[BUFSIZ];
    unsigned long now;

    if (gethostname(hostnamebuf, sizeof(hostnamebuf) - 1) < 0)
	error("gethostname: %m");
    hostnamebuf[sizeof(hostnamebuf) - 1] = 0;
    now = time((time_t *) 0);

    /*
     * Identify table type and table origin.
     */
    printf("class|host|device|start_time\n");
    printf("ils|%s|%s|%lu\n", hostnamebuf, device, now);

    /*
     * Identify the fields in the data that follow.
     */
    printf("st_ino|st_alloc|st_uid|st_gid|st_mtime|st_atime|st_ctime");
#ifdef HAVE_DTIME
    printf("|st_dtime");
#endif
    printf("|st_mode|st_nlink|st_size|st_block0|st_block1\n");
}

/* print_inode - list generic inode */

static void print_inode(INUM_T inum, FS_INODE *fs_inode, int flags,
			        char *unused_context)
{
    printf("%lu|%c|%d|%d|%lu|%lu|%lu",
	   (ULONG) inum, (flags & FS_FLAG_ALLOC) ? 'a' : 'f',
	   (int) fs_inode->uid, (int) fs_inode->gid,
	   (ULONG) fs_inode->mtime, (ULONG) fs_inode->atime,
	   (ULONG) fs_inode->ctime);
#ifdef HAVE_DTIME
    printf("|%lu", (ULONG) fs_inode->dtime);
#endif
    if (sizeof(fs_inode->size) <= sizeof(unsigned long))
	printf("|%lo|%d|%lu|%lu|%lu\n",
	       (ULONG) fs_inode->mode, (int) fs_inode->nlink,
	       (ULONG) fs_inode->size, (ULONG) fs_inode->direct_addr[0],
	       (ULONG) fs_inode->direct_addr[1]);
    else
	printf("|%lo|%d|%llu|%lu|%lu\n",
	       (ULONG) fs_inode->mode, (int) fs_inode->nlink,
	       (unsigned long long) fs_inode->size, (ULONG) fs_inode->direct_addr[0],
	       (ULONG) fs_inode->direct_addr[1]);
}

/* main - open file system, list inode info */

int     main(int argc, char **argv)
{
    FS_INFO *fs;
    char   *start;
    char   *last;
    INUM_T  istart;
    INUM_T  ilast;
    int     ch;
    int     flags = 0;
    char   *fstype = DEF_FSTYPE;

    progname = argv[0];

    /*
     * Provide convenience options for the most commonly selected feature
     * combinations.
     */
    while ((ch = getopt(argc, argv, "aAef:lLorvVzZ")) > 0) {
	switch (ch) {
	default:
	    usage();
	case 'e':
	    flags |= ~0;
	    break;
	case 'f':
	    fstype = optarg;
	    break;
	case 'o':
	    flags |= (FS_FLAG_ALLOC | FS_FLAG_UNLINK);
	    break;
	case 'r':
	    flags |= (FS_FLAG_UNLINK | FS_FLAG_USED);
	    break;
	case 'v':
	    verbose++;
	    logfp = stderr;
	    break;
	case 'V':
	    verbose++;
	    logfp = stdout;
	    break;

	    /*
	     * Provide fine controls to tweak one feature at a time.
	     */
	case 'a':
	    flags |= FS_FLAG_ALLOC;
	    flags &= ~FS_FLAG_UNALLOC;
	    break;
	case 'A':
	    flags |= FS_FLAG_UNALLOC;
	    flags &= ~FS_FLAG_ALLOC;
	    break;
	case 'l':
	    flags |= FS_FLAG_LINK;
	    flags &= ~FS_FLAG_UNLINK;
	    break;
	case 'L':
	    flags |= FS_FLAG_UNLINK;
	    flags &= ~FS_FLAG_LINK;
	    break;
	case 'z':
	    flags |= FS_FLAG_UNUSED;
	    flags &= ~FS_FLAG_USED;
	    break;
	case 'Z':
	    flags |= FS_FLAG_USED;
	    flags &= ~FS_FLAG_UNUSED;
	    break;
	}
    }

    if (optind >= argc)
	usage();

    /*
     * Apply rules for default settings. Assume a "don't care" condition when
     * nothing is explicitly selected from a specific feature category.
     */
    if (flags == 0)
	flags = DEF_FLAGS;
    if ((flags & (FS_FLAG_ALLOC | FS_FLAG_UNALLOC)) == 0)
	flags |= FS_FLAG_ALLOC | FS_FLAG_UNALLOC;
    if ((flags & (FS_FLAG_LINK | FS_FLAG_UNLINK)) == 0)
	flags |= FS_FLAG_LINK | FS_FLAG_UNLINK;
    if ((flags & (FS_FLAG_USED | FS_FLAG_UNUSED)) == 0)
	flags |= FS_FLAG_USED | FS_FLAG_UNUSED;

    /*
     * Open the file system.
     */
    fs = fs_open(argv[optind], fstype);

    /*
     * Print the time machine header.
     */
    print_header(argv[optind]);

    /*
     * List the named inodes.
     */
    optind++;
    if (optind < argc) {
	while ((start = argv[optind]) != 0) {
	    last = split_at(start, '-');
	    istart = (*start ? atoinum(start) : fs->start_inum);
	    ilast = (!last ? istart : *last ? atoinum(last) : fs->last_inum);
	    fs->inode_walk(fs, istart, ilast, flags, print_inode, (char *) 0);
	    optind++;
	}
    }

    /*
     * List all inodes, subject to the specified restrictions.
     */
    else {
	fs->inode_walk(fs, fs->start_inum, fs->last_inum,
		       flags, print_inode, (char *) 0);
    }
    fs->close(fs);
    exit(0);
}

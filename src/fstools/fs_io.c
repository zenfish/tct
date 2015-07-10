/*++
/* NAME
/*	fs_io 3
/* SUMMARY
/*	file system I/O routines
/* SYNOPSIS
/*	#include "fstools.h"
/*
/*	void	fs_read_block(fs, buf, len, addr, comment)
/*	FS_INFO *fs;
/*	FS_BUF	*buf;
/*	int	len;
/*	DADDR_T	addr;
/*	const char *comment;
/*
/*	void	fs_read_random(fs, buf, len, offs, comment)
/*	FS_INFO *fs;
/*	char	*buf;
/*	int	len;
/*	OFF_T	offs;
/*	const char *comment;
/* DESCRIPTION
/*	fs_read_block() reads a block of data from the named file system.
/*
/*	fs_read_random() reads from an arbitrary position.
/*
/*	Arguments:
/* .IP fs
/*	File system handle.
/* .IP buf
/*	Result buffer pointer.
/* .IP len
/*	Amount of bytes to be read.
/* .IP addr
/*	Block number.
/* .IP fd
/*	Open device file.
/* .IP offs
/*	Byte offset.
/* .IP comment
/*	Text logged with verbose logging.
/* LICENSE
/*	This software is distributed under the IBM Public License.
/* AUTHOR(S)
/*	Wietse Venema
/*	IBM T.J. Watson Research
/*	P.O. Box 704
/*	Yorktown Heights, NY 10598, USA
/*--*/

#include "fs_tools.h"
#include "mymalloc.h"
#include "error.h"

/* fs_read_block - read a block from a file system */

void    fs_read_block(FS_INFO *fs, FS_BUF *buf, int len, DADDR_T addr,
		              const char *comment)
{
    char   *myname = "fs_read_block";
    OFF_T   offs;

    /*
     * Sanity checks.
     */
    if (len % DEV_BSIZE)
	panic("%s: len %d not multiple of %d", myname, len, DEV_BSIZE);
    if (len > buf->size)
	panic("%s: len %d > %d", myname, len, buf->size);

    /*
     * Bummer: with the 32-bit file model, Solaris 7 read() won't go past 2GB
     * seek offsets, so we must use pread() instead of read(). When printing
     * disk offsets, convert to double in case they have huge file offset
     * types and the number does not fit in an unsigned long.
     */
    buf->addr = addr;
    offs = (OFF_T) addr *fs->block_size;

    if (verbose)
	fprintf(logfp, "%s: read block %lu offs %.0f len %d (%s)\n",
		myname, (ULONG) addr, (double) offs, len, comment);
#ifdef USE_PREAD
    if (pread(fs->fd, buf->data, len, offs) != len)
	error("read (%d@%.0f): %m", len, (double) offs);
#else
    if (fs->seek_pos != offs)
	if (LSEEK(fs->fd, offs, SEEK_SET) != offs)
	    error("lseek (%.0f): %m", (double) offs);
    if (read(fs->fd, buf->data, len) != len)
	error("read (%d@%.0f): %m", len, (double) offs);
#endif
    fs->seek_pos = offs + len;
    buf->used = len;
}

/* fs_read_random - random-access read */

void    fs_read_random(FS_INFO *fs, char *buf, int len, OFF_T offs,
		               const char *comment)
{
    char   *myname = "fs_read_random";
    int     count;

    if (verbose)
	fprintf(logfp, "%s: read offs %.0f len %d (%s)\n",
		myname, (double) offs, len, comment);
    if (LSEEK(fs->fd, offs, SEEK_SET) != offs)
	error("seek offset %lu: %m", (ULONG) offs);
    if ((count = read(fs->fd, buf, len)) != len)
	error("read: %m");

    fs->seek_pos = offs + len;
}

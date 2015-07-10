/*++
/* NAME
/*	fs_copy_file 3
/* SUMMARY
/*	copy a file by inode
/* SYNOPSIS
/*	#include "fstools.h"
/*
/*	void	fs_copy_file(FS_INFO *fs, INUM_T inum, int flags)
/* DESCRIPTION
/*	fs_copy_file() copies the contents of the file with the
/*	specified inode number in the specified file system to
/*	the standard output stream. The flags argument specifies
/*	a restriction on the blocks to be copied, and is the 
/*	bit-wise OR of:
/* .IP FS_FLAG_ALLOC 
/*	Copy allocated blocks.
/* .IP FS_FLAG_UNALLOC
/*	Copy unallocated blocks.
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

/* fs_copy_direct - copy direct block */

static int fs_copy_direct(FS_INFO *fs, FS_BUF *buf[],
			          OFF_T length, DADDR_T addr, int flags)
{
    int     read_count;

    read_count = (length < buf[0]->size ? length : buf[0]->size);
    if (addr == 0) {
	if (flags & FS_FLAG_UNALLOC) {
	    memset(buf[0]->data, 0, read_count);
	    if (fwrite(buf[0]->data, read_count, 1, stdout) != 1)
		error("write: %m");
	}
    } else {
	if (flags & FS_FLAG_ALLOC) {
	    fs_read_block(fs, buf[0], roundup(read_count, DEV_BSIZE), addr,
			  "data block");
	    if (fwrite(buf[0]->data, read_count, 1, stdout) != 1)
		error("write: %m");
	}
    }
    return (read_count);
}

/* fs_copy_indir - copy indirect block */

static int fs_copy_indir(FS_INFO *fs, FS_BUF *buf[], OFF_T length,
			         DADDR_T addr, int level, int flags)
{
    char   *myname = "fs_copy_indir";
    OFF_T   todo_count = length;
    DADDR_T *iaddr;
    int     n;

    if (verbose)
	fprintf(logfp, "%s: level %d block %lu\n", myname, level, (ULONG) addr);

    /*
     * Read a block of disk addresses.
     */
    if (addr == 0)
	memset(buf[level]->data, 0, buf[level]->size);
    else
	fs_read_block(fs, buf[level], buf[level]->size, addr,
		      "disk address block");

    /*
     * For each disk address, copy a direct block or process an indirect
     * block.
     */
    iaddr = (DADDR_T *) buf[level]->data;
    for (n = 0; todo_count > 0 && n < buf[level]->size / sizeof(*iaddr); n++)
	if (level == 1)
	    todo_count -= fs_copy_direct(fs, buf, todo_count, iaddr[n], flags);
	else
	    todo_count -= fs_copy_indir(fs, buf, todo_count, iaddr[n],
					level - 1, flags);

    return (length - todo_count);
}

/* fs_copy_file - copy file by inode */

void    fs_copy_file(FS_INFO *fs, INUM_T inum, int flags)
{
    FS_INODE *inode;
    OFF_T   length;
    FS_BUF **buf;
    int     n;
    int     level;

    /*
     * Read the on-disk inode.
     */
    inode = fs->inode_lookup(fs, inum);

    /*
     * Initialize a buffer for each level of indirection that is supported by
     * this inode. The level 0 buffer is sized to the logical block size used
     * for files. The level 1.. buffers are sized to the block size used for
     * indirect blocks.
     */
    buf = (FS_BUF **) mymalloc(sizeof(*buf) * (inode->indir_count + 1));
    buf[0] = fs_buf_alloc(fs->file_bsize);
    for (level = 1; level <= inode->indir_count; level++)
	buf[level] = fs_buf_alloc(fs->addr_bsize);

    /*
     * Read the file blocks. First the direct blocks, then the indirect ones.
     */
    length = inode->size;
    for (n = 0; length > 0 && n < inode->direct_count; n++)
	length -= fs_copy_direct(fs, buf, length, inode->direct_addr[n], flags);

    for (level = 1; length > 0 && level <= inode->indir_count; level++)
	length -= fs_copy_indir(fs, buf, length, inode->indir_addr[level - 1],
				level, flags);

    /*
     * Cleanup.
     */
    for (level = 0; level <= inode->indir_count; level++)
	fs_buf_free(buf[level]);
    free((char *) buf);
    fs_inode_free(inode);
}

/*++
/* NAME
/*	fs_inode 3
/* SUMMARY
/*	generic inode support
/* SYNOPSIS
/*	#include "fstools.h"
/*
/*	FS_INODE *fs_inode_alloc(direct_count, indir_count)
/*	int	direct_count;
/*	int	indir_count;
/*
/*	FS_INODE *fs_inode_realloc(fs_inode, direct_count, indir_count)
/*	FS_INODE *fs_inode;
/*	int	direct_count;
/*	int	indir_count;
/*
/*	void	fs_inode_free(fs_inode)
/*	FS_INODE *fs_inode;
/* DESCRIPTION
/*	fs_inode_alloc() allocates storage for inode information with
/*	\fBdirect_count\fR direct block pointers and \fBindir_count\fR
/*	indirect block pointers.
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

/* fs_inode_alloc - allocate generic inode structure */

FS_INODE *fs_inode_alloc(int direct_count, int indir_count)
{
    FS_INODE *fs_inode;

    fs_inode = (FS_INODE *) mymalloc(sizeof(*fs_inode));

    fs_inode->direct_count = direct_count;
    fs_inode->direct_addr =
	(DADDR_T *) mymalloc(direct_count * sizeof(DADDR_T));

    fs_inode->indir_count = indir_count;
    fs_inode->indir_addr =
	(DADDR_T *) mymalloc(indir_count * sizeof(DADDR_T));

    return (fs_inode);
}

/* fs_inode_realloc - resize generic inode structure */

FS_INODE *fs_inode_realloc(FS_INODE *fs_inode, int direct_count, int indir_count)
{
    if (fs_inode->direct_count != direct_count) {
	fs_inode->direct_count = direct_count;
	fs_inode->direct_addr =
	    (DADDR_T *) myrealloc((char *) fs_inode->direct_addr,
				  direct_count * sizeof(DADDR_T));
    }
    if (fs_inode->indir_count != indir_count) {
	fs_inode->indir_count = indir_count;
	fs_inode->indir_addr =
	    (DADDR_T *) myrealloc((char *) fs_inode->indir_addr,
				  indir_count * sizeof(DADDR_T));
    }
    return (fs_inode);
}

/* fs_inode_free - destroy generic inode structure */

void    fs_inode_free(FS_INODE *fs_inode)
{
    if (fs_inode->direct_addr)
	free((char *) fs_inode->direct_addr);
    if (fs_inode->indir_addr)
	free((char *) fs_inode->indir_addr);
    free((char *) fs_inode);
}

/*++
/* NAME
/*	ffs_open 3
/* SUMMARY
/*	fast file system support
/* SYNOPSIS
/*	#include "fstools.h"
/*
/*	FS_INFO *ffs_open(const char *name)
/* DESCRIPTION
/*	ffs_open() opens the named block device and makes it accessible
/*	for the standard file system operations described in fs_open(3).
/*	This module supports both UFS1 (traditional FFS) and UFS2 (FFS
/*	with expanded range).
/* BUGS
/*	On-disk layout and byte order differ per FFS implementation,
/*	therefore this code is likely to fail when confronted with
/*	foreign file systems.
/* LICENSE
/*	This software is distributed under the IBM Public License.
/* AUTHOR(S)
/*	Wietse Venema
/*	IBM T.J. Watson Research
/*	P.O. Box 704
/*	Yorktown Heights, NY 10598, USA
/*--*/

#include "fs_tools.h"
#ifdef FS_UFS2_MAGIC
#include "mymalloc.h"
#include "error.h"

 /*
  * Structure of a fast file system handle.
  */
union dinode {
    struct ufs1_dinode u1;		/* UFS1 disk inode */
    struct ufs2_dinode u2;		/* UFS2 disk inode */
};

typedef struct {
    FS_INFO fs_info;			/* super class */
    struct fs *fs;			/* super block buffer */
    FS_BUF *cg_buf;			/* cylinder block buffer */
    FS_BUF *dino_buf;			/* inode block buffer */
    union dinode dinode;		/* disk inode */
} FFS_INFO;

/* ffs_cgroup_lookup - look up cached cylinder group info */

static struct cg *ffs_cgroup_lookup(FFS_INFO *ffs, CGNUM_T cgnum)
{
    struct cg *cg;
    DADDR_T addr;

    /*
     * Sanity check
     */
    if (cgnum < 0 || cgnum >= ffs->fs->fs_ncg)
	error("invalid cylinder group number: %lu", (ULONG) cgnum);

    /*
     * Allocate/read cylinder group info on the fly. Trust that a cylinder
     * group always fits within a logical disk block (as promised in the
     * 4.4BSD <ufs/ffs/fs.h> include file).
     */
    addr = cgtod(ffs->fs, cgnum);
    if (ffs->cg_buf == 0)
	ffs->cg_buf = fs_buf_alloc(ffs->fs->fs_bsize);
    cg = (struct cg *) ffs->cg_buf->data;
    if (ffs->cg_buf->addr != addr) {
	fs_read_block(&ffs->fs_info, ffs->cg_buf, ffs->cg_buf->size, addr,
		      "cylinder block");
	if (verbose)
	    fprintf(logfp,
		  "\tcyl group %lu: %lu/%lu/%lu free blocks/inodes/frags\n",
		    (ULONG) cgnum, (ULONG) cg->cg_cs.cs_nbfree,
		    (ULONG) cg->cg_cs.cs_nifree,
		    (ULONG) cg->cg_cs.cs_nffree);
    }
    return (cg);
}

/* ffs_cgroup_free - destroy cylinder group info cache */

static void ffs_cgroup_free(FFS_INFO *ffs)
{
    if (ffs->cg_buf)
	fs_buf_free(ffs->cg_buf);
}

/* ffs_dinode_lookup - look up cached disk inode */

static union dinode *ffs_dinode_lookup(FFS_INFO *ffs, INUM_T inum)
{
    DADDR_T addr;
    int     offs;

    /*
     * Sanity check.
     */
    if (inum < ffs->fs_info.start_inum || inum > ffs->fs_info.last_inum)
	error("invalid inode number: %lu", (ULONG) inum);

    /*
     * Allocate/read the inode buffer on the fly.
     */
    if (ffs->dino_buf == 0)
	ffs->dino_buf = fs_buf_alloc(ffs->fs->fs_bsize);
    addr = itod(ffs->fs, inum);
    if (ffs->dino_buf->addr != addr)
	fs_read_block(&ffs->fs_info, ffs->dino_buf, ffs->dino_buf->size, addr,
		      "inode block");

    /*
     * Copy the inode, in order to avoid alignment problems when accessing
     * structure members.
     */
    if (ffs->fs->fs_magic == FS_UFS2_MAGIC) {
	offs = itoo(ffs->fs, inum) * sizeof(struct ufs2_dinode);
	memcpy((char *) &ffs->dinode.u2, ffs->dino_buf->data + offs,
	       sizeof(struct ufs2_dinode));
    } else {
	offs = itoo(ffs->fs, inum) * sizeof(struct ufs1_dinode);
	memcpy((char *) &ffs->dinode.u1, ffs->dino_buf->data + offs,
	       sizeof(struct ufs1_dinode));
    }
    return (&ffs->dinode);
}

/* ffs_dinode_free - destroy disk inode cache */

static void ffs_dinode_free(FFS_INFO *ffs)
{
    if (ffs->dino_buf)
	fs_buf_free(ffs->dino_buf);
}

/* ffs_copy_inode - copy disk inode to generic inode */

static void ffs_copy_inode(struct fs * fs, union dinode * dino, FS_INODE *fs_inode)
{
    int     i;

    if (fs->fs_magic == FS_UFS2_MAGIC) {
	fs_inode->mode = dino->u2.di_mode;
	fs_inode->nlink = dino->u2.di_nlink;
	fs_inode->size = dino->u2.di_size;
	fs_inode->uid = dino->u2.di_uid;
	fs_inode->gid = dino->u2.di_gid;
	fs_inode->mtime = INOTIME(dino->u2.di_mtime);
	fs_inode->atime = INOTIME(dino->u2.di_atime);
	fs_inode->ctime = INOTIME(dino->u2.di_ctime);
	if (fs_inode->direct_count != NDADDR || fs_inode->indir_count != NIADDR)
	    fs_inode_realloc(fs_inode, NDADDR, NIADDR);
	for (i = 0; i < NDADDR; i++)
	    fs_inode->direct_addr[i] = dino->u2.di_db[i];
	for (i = 0; i < NIADDR; i++)
	    fs_inode->indir_addr[i] = dino->u2.di_ib[i];
    } else {
	fs_inode->mode = dino->u1.di_mode;
	fs_inode->nlink = dino->u1.di_nlink;
	fs_inode->size = dino->u1.di_size;
	fs_inode->uid = dino->u1.di_uid;
	fs_inode->gid = dino->u1.di_gid;
	fs_inode->mtime = INOTIME(dino->u1.di_mtime);
	fs_inode->atime = INOTIME(dino->u1.di_atime);
	fs_inode->ctime = INOTIME(dino->u1.di_ctime);
	if (fs_inode->direct_count != NDADDR || fs_inode->indir_count != NIADDR)
	    fs_inode_realloc(fs_inode, NDADDR, NIADDR);
	for (i = 0; i < NDADDR; i++)
	    fs_inode->direct_addr[i] = dino->u1.di_db[i];
	for (i = 0; i < NIADDR; i++)
	    fs_inode->indir_addr[i] = dino->u1.di_ib[i];
    }
}

/* ffs_inode_lookup - lookup inode, external interface */

static FS_INODE *ffs_inode_lookup(FS_INFO *fs, INUM_T inum)
{
    FFS_INFO *ffs = (FFS_INFO *) fs;
    FS_INODE *fs_inode = fs_inode_alloc(NDADDR, NIADDR);
    union dinode *dino = ffs_dinode_lookup(ffs, inum);

    ffs_copy_inode(ffs->fs, dino, fs_inode);
    return (fs_inode);
}

/* ffs_inode_walk - inode iterator */

void    ffs_inode_walk(FS_INFO *fs, INUM_T start, INUM_T last, int flags,
		               FS_INODE_WALK_FN action, char *ptr)
{
    char   *myname = "ffs_inode_walk";
    FFS_INFO *ffs = (FFS_INFO *) fs;
    CGNUM_T cg_num;
    struct cg *cg = 0;
    INUM_T  inum;
    unsigned char *inosused;
    union dinode *dino;
    FS_INODE *fs_inode = fs_inode_alloc(NDADDR, NIADDR);
    int     myflags;
    INUM_T  ibase;

    /*
     * Sanity checks.
     */
    if (start < ffs->fs_info.start_inum || start > ffs->fs_info.last_inum)
	error("%s: invalid start inode number: %lu", myname, (ULONG) start);
    if (last < ffs->fs_info.start_inum || last > ffs->fs_info.last_inum)
	error("%s: invalid last inode number: %lu", myname, (ULONG) last);

    /*
     * Iterate. This is easy because inode numbers are contiguous, unlike
     * data blocks which are interleaved with cylinder group blocks.
     */
    for (inum = start; inum <= last; inum++) {

	/*
	 * Be sure to use the proper cylinder group data.
	 */
	cg_num = INO_TO_CG(ffs->fs, inum);
	if (cg == 0 || cg->cg_cgx != cg_num) {
	    cg = ffs_cgroup_lookup(ffs, cg_num);
	    inosused = (unsigned char *) cg_inosused(cg);
	    ibase = cg_num * ffs->fs->fs_ipg;
	}

	/*
	 * Apply the allocated/unallocated restriction.
	 */
	myflags = (isset(inosused, inum - ibase) ?
		   FS_FLAG_ALLOC : FS_FLAG_UNALLOC);
	if ((flags & myflags) != myflags)
	    continue;

	/*
	 * Apply the linked/unlinked restriction.
	 */
#define UFS_DINO_FIELD(fs, dino, field) ((fs)->fs_magic == FS_UFS2_MAGIC ? \
	    dino->u2.field : dino->u1.field)

	dino = ffs_dinode_lookup(ffs, inum);
	myflags |= (UFS_DINO_FIELD(ffs->fs, dino, di_nlink) ?
		    FS_FLAG_LINK : FS_FLAG_UNLINK);
	if ((flags & myflags) != myflags)
	    continue;

	/*
	 * Apply the used/unused restriction.
	 */
	myflags |= (INOTIME(UFS_DINO_FIELD(ffs->fs, dino, di_ctime)) ?
		    FS_FLAG_USED : FS_FLAG_UNUSED);
	if ((flags & myflags) != myflags)
	    continue;

	/*
	 * Fill in a file system-independent inode structure and pass control
	 * to the application.
	 */
	ffs_copy_inode(ffs->fs, dino, fs_inode);
	action(inum, fs_inode, myflags, ptr);
    }

    /*
     * Cleanup.
     */
    fs_inode_free(fs_inode);
}

/* ffs_block_walk - block iterator */

void    ffs_block_walk(FS_INFO *fs, DADDR_T start, DADDR_T last, int flags,
		               FS_BLOCK_WALK_FN action, char *ptr)
{
    char   *myname = "ffs_block_walk";
    FFS_INFO *ffs = (FFS_INFO *) fs;
    FS_BUF *fs_buf = fs_buf_alloc(fs->block_size * fs->block_frags);
    CGNUM_T cg_num;
    struct cg *cg = 0;
    DADDR_T dbase;
    DADDR_T dmin;			/* first data block in group */
    DADDR_T sblock;			/* super block in group */
    DADDR_T addr;
    DADDR_T faddr;
    unsigned char *freeblocks;
    int     myflags;
    int     want;
    int     frags;
    char   *null_block;

    /*
     * Sanity checks.
     */
    if (start < fs->start_block || start > fs->last_block)
	error("%s: invalid start block number: %lu", myname, (ULONG) start);
    if (last < fs->start_block || last > fs->last_block)
	error("%s: invalid last block number: %lu", myname, (ULONG) last);
    if ((flags & FS_FLAG_ALIGN) && (start % fs->block_frags) != 0)
	error("%s: specify -b or specify block-aligned start block", myname);

    /*
     * Other initialization.
     */
    if (flags & FS_FLAG_ALIGN) {
	null_block = mymalloc(fs->block_size);
	memset(null_block, 0, fs->block_size);
    }

    /*
     * Iterate. This is not as tricky as it could be, because the free list
     * map covers the entire disk partition, including blocks occupied by
     * cylinder group maps, boot blocks, and other non-data blocks.
     * 
     * Examine the disk one logical block at a time. A logical block may be
     * composed of a number of fragment blocks. For example, the 4.4BSD
     * filesystem has logical blocks of 8 fragments.
     */
    for (addr = start; addr <= last; addr += fs->block_frags) {

	/*
	 * Be sure to use the right cylinder group information.
	 */
	cg_num = dtog(ffs->fs, addr);
	if (cg == 0 || cg->cg_cgx != cg_num) {
	    cg = ffs_cgroup_lookup(ffs, cg_num);
	    freeblocks = (unsigned char *) cg_blksfree(cg);
	    dbase = cgbase(ffs->fs, cg_num);
	    dmin = cgdmin(ffs->fs, cg_num);
	    sblock = cgsblock(ffs->fs, cg_num);
	}
	if (addr < dbase)
	    remark("impossible: cyl group %lu: block %lu < cgbase %lu",
		   (unsigned long) cg_num, (unsigned long) addr,
		   (unsigned long) dbase);

	/*
	 * Prepare for file systems that have a partial last logical block.
	 */
	frags = (last + 1 - addr > fs->block_frags ?
		 fs->block_frags : last + 1 - addr);

	/*
	 * See if this logical block contains any fragments of interest. If
	 * not, skip the entire logical block.
	 */
	for (want = 0, faddr = addr; want == 0 && faddr < addr + frags; faddr++)
	    want = (flags & (isset(freeblocks, faddr - dbase) ?
			     FS_FLAG_UNALLOC : FS_FLAG_ALLOC));
	if (want == 0)
	    continue;

	/*
	 * Pass blocks of interest to the application, optionally padding the
	 * data with null blocks in order to maintain logical block
	 * alignment.
	 * 
	 * Beware: FFS stores file data in the blocks between the start of a
	 * cylinder group and the start of its super block.
	 */
	for (faddr = addr; faddr < addr + frags; faddr++) {
	    myflags = (isset(freeblocks, faddr - dbase) ?
		       FS_FLAG_UNALLOC : FS_FLAG_ALLOC);
	    if (faddr >= sblock && faddr < dmin)
		myflags |= FS_FLAG_META;
	    if ((myflags & FS_FLAG_META) && (myflags & FS_FLAG_UNALLOC))
		remark("impossible: unallocated meta block %lu!!",
		       (unsigned long) faddr);
	    if ((flags & myflags) != myflags) {
		if (flags & FS_FLAG_ALIGN)
		    action(faddr, null_block, myflags, ptr);
	    } else {
		if (fs_buf->addr < 0
		    || faddr >= fs_buf->addr + fs->block_frags)
		    fs_read_block(fs, fs_buf, fs->block_size * frags, addr,
				  "data block");
		action(faddr,
		     fs_buf->data + fs->block_size * (faddr - fs_buf->addr),
		       myflags, ptr);
	    }
	}
    }

    /*
     * Cleanup.
     */
    if (flags & FS_FLAG_ALIGN)
	free(null_block);
    fs_buf_free(fs_buf);
}

/* ffs_close - close a fast file system */

static void ffs_close(FS_INFO *fs)
{
    FFS_INFO *ffs = (FFS_INFO *) fs;

    close(ffs->fs_info.fd);
    ffs_cgroup_free(ffs);
    ffs_dinode_free(ffs);
    free((char *) ffs->fs);
    free(ffs);
}

/* ffs_open - open a fast file system */

FS_INFO *ffs_open(const char *name)
{
    char   *myname = "ffs_open";
    FFS_INFO *ffs = (FFS_INFO *) mymalloc(sizeof(*ffs));
    static off_t sblock_offs[] = SBLOCKSEARCH;
    off_t  *sp;

    /*
     * Open the raw device and read the superblock. We must use a read buffer
     * that is a multiple of the physical blocksize, because attempts to read
     * a partial physical block can fail. XXX How does one know the physical
     * blocksize without reading the disk? One assumes.
     */
    if ((ffs->fs_info.fd = open(name, O_RDONLY)) < 0)
	error("%s: open %s: %m", myname, name);

    /*
     * Read the superblock.
     */
    ffs->fs = (struct fs *) mymalloc(SBLOCKSIZE);
    for (sp = sblock_offs; /* see below */ ; sp++) {
	if (*sp < 0)
	    error("%s: no recognizable superblock found", name);
	if (verbose)
	    remark("trying: offset %ld", (long) *sp);
	if (LSEEK(ffs->fs_info.fd, *sp, SEEK_SET) != *sp)
	    error("%s: lseek: %m", myname);
	if (read(ffs->fs_info.fd, (char *) ffs->fs, SBLOCKSIZE) != SBLOCKSIZE)
	    error("%s: read superblock: %m", name);
	if (ffs->fs->fs_magic == FS_UFS2_MAGIC
	    || ffs->fs->fs_magic == FS_UFS1_MAGIC)
	    break;
    }
    if (verbose)
	remark("UFS%d file system", ffs->fs->fs_magic == FS_UFS2_MAGIC ?
	       2 : 1);

    /*
     * Translate some filesystem-specific information to generic form.
     */
    ffs->fs_info.inum_count = ffs->fs->fs_ncg * ffs->fs->fs_ipg;
    ffs->fs_info.start_inum = 0;
    ffs->fs_info.last_inum = ffs->fs_info.inum_count - 1;
    ffs->fs_info.block_count = (ffs->fs->fs_magic == FS_UFS2_MAGIC ?
				ffs->fs->fs_size : ffs->fs->fs_old_size);
    ffs->fs_info.start_block = 0;
    ffs->fs_info.last_block = ffs->fs_info.block_count - 1;
    ffs->fs_info.block_size = ffs->fs->fs_fsize;
    ffs->fs_info.file_bsize = ffs->fs->fs_bsize;
    ffs->fs_info.addr_bsize = ffs->fs->fs_bsize;
    ffs->fs_info.block_frags = ffs->fs->fs_frag;

    /*
     * Other initialization: caches, callbacks.
     */
    ffs->cg_buf = 0;
    ffs->dino_buf = 0;
    ffs->fs_info.seek_pos = -1;
    ffs->fs_info.inode_walk = ffs_inode_walk;
    ffs->fs_info.block_walk = ffs_block_walk;
    ffs->fs_info.inode_lookup = ffs_inode_lookup;
    ffs->fs_info.close = ffs_close;

    /*
     * Print some stats.
     */
    if (verbose)
	fprintf(logfp,
		"inodes %lu root ino %lu cyl groups %lu blocks %lu\n",
		(ULONG) ffs->fs->fs_ncg * ffs->fs->fs_ipg,
		(ULONG) ROOTINO,
		(ULONG) ffs->fs->fs_ncg,
		(ULONG) ffs->fs_info.block_count);
    return (&ffs->fs_info);
}

#endif

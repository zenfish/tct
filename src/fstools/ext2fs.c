/*++
/* NAME
/*	ext2fs_open 3
/* SUMMARY
/*	LINUX file system support
/* SYNOPSIS
/*	#include "fstools.h"
/*
/*	FS_INFO *ext2fs_open(const char *name)
/* DESCRIPTION
/*	ext2fs_open() opens the named block device and makes it accessible
/*	for the standard file system operations described in fs_open(3).
/* BUGS
/*	You need a LINUX machine in order to access LINUX disks.
/* LICENSE
/*	This software is distributed under the IBM Public License.
/* AUTHOR(S)
/*	Wietse Venema
/*	IBM T.J. Watson Research
/*	P.O. Box 704
/*	Yorktown Heights, NY 10598, USA
/*--*/

#include "fs_tools.h"
#ifdef HAVE_EXT2FS
#include "mymalloc.h"
#include "error.h"

 /*
  * Structure of an ext2fs file system handle.
  */
typedef struct {
    FS_INFO fs_info;			/* super class */
    struct ext2_super_block fs;		/* super block buffer */
    struct ext2_group_desc group;	/* cached group descriptor */
    GRPNUM_T grpnum;			/* cached group number */
    UCHAR  *block_map;			/* cached block allocation bitmap */
    GRPNUM_T bmap_num;			/* cached block bitmap nr */
    UCHAR  *inode_map;			/* cached inode allocation bitmap */
    GRPNUM_T imap_num;			/* cached inode bitmap nr */
    struct ext2_inode inode;		/* cached disk inode */
    INUM_T  inum;			/* cached inode number */
    OFF_T   group_offset;		/* offset to first group desc */
    int     groups_count;		/* nr of descriptor group blocks */
} EXT2FS_INFO;

/* ext2fs_group_lookup - look up group descriptor info */

static struct ext2_group_desc *ext2fs_group_lookup(EXT2FS_INFO *ext2fs, GRPNUM_T grpnum)
{
    struct ext2_group_desc *gd = &ext2fs->group;
    OFF_T   offs;

    /*
     * Sanity check
     */
    if (grpnum < 0 || grpnum >= ext2fs->groups_count)
	error("invalid group descriptor number: %lu", (ULONG) grpnum);

    /*
     * We're not reading group descriptors often, so it is OK to do small
     * reads instead of cacheing group descriptors in a large buffer.
     */
    offs = ext2fs->group_offset + grpnum * sizeof(*gd);
    fs_read_random(&ext2fs->fs_info, (char *) gd, sizeof(*gd), offs,
		   "group descriptor");
    ext2fs->grpnum = grpnum;
    if (verbose)
	fprintf(logfp,
		"\tgroup %lu: %lu/%lu free blocks/inodes\n",
		(ULONG) grpnum,
		(ULONG) gd->bg_free_blocks_count,
		(ULONG) gd->bg_free_inodes_count);
    return (gd);
}

/* ext2fs_print_map - print a bitmap */

static void ext2fs_print_map(UCHAR * map, int len)
{
    int     i;

    for (i = 0; i < len; i++) {
	if (i > 0 && i % 10 == 0)
	    putc('|', logfp);
	putc(isset(map, i) ? '1' : '.', logfp);
    }
    putc('\n', logfp);
}

/* ext2fs_bmap_lookup - look up block bitmap */

UCHAR  *ext2fs_bmap_lookup(EXT2FS_INFO *ext2fs, GRPNUM_T grpnum)
{
    struct ext2_group_desc *gd = &ext2fs->group;

    /*
     * Look up the group descriptor info.
     */
    if (ext2fs->grpnum != grpnum)
	ext2fs_group_lookup(ext2fs, grpnum);

    /*
     * Look up the block allocation bitmap.
     */
    fs_read_random(&ext2fs->fs_info,
		   (char *) ext2fs->block_map,
		   ext2fs->fs_info.block_size,
		   (OFF_T) gd->bg_block_bitmap * ext2fs->fs_info.block_size,
		   "block bitmap");
    ext2fs->bmap_num = grpnum;
    if (verbose > 1)
	ext2fs_print_map(ext2fs->block_map, ext2fs->fs.s_blocks_per_group);

    return (ext2fs->block_map);
}

/* ext2fs_imap_lookup - look up inode bitmap */

UCHAR  *ext2fs_imap_lookup(EXT2FS_INFO *ext2fs, GRPNUM_T grpnum)
{
    struct ext2_group_desc *gd = &ext2fs->group;

    /*
     * Look up the group descriptor info.
     */
    if (ext2fs->grpnum != grpnum)
	ext2fs_group_lookup(ext2fs, grpnum);

    /*
     * Look up the inode allocation bitmap.
     */
    fs_read_random(&ext2fs->fs_info,
		   (char *) ext2fs->inode_map,
		   ext2fs->fs_info.block_size,
		   (OFF_T) gd->bg_inode_bitmap * ext2fs->fs_info.block_size,
		   "inode bitmap");
    ext2fs->imap_num = grpnum;
    if (verbose > 1)
	ext2fs_print_map(ext2fs->inode_map, ext2fs->fs.s_inodes_per_group);

    return (ext2fs->inode_map);
}

/* ext2fs_dinode_lookup - look up disk inode */

static struct ext2_inode *ext2fs_dinode_lookup(EXT2FS_INFO *ext2fs, INUM_T inum)
{
    struct ext2_group_desc *gd = &ext2fs->group;
    struct ext2_inode *dino = &ext2fs->inode;
    GRPNUM_T grpnum;
    OFF_T   addr;
    size_t  offs;

    /*
     * Sanity check.
     */
    if (inum < ext2fs->fs_info.start_inum || inum > ext2fs->fs.s_inodes_count)
	error("invalid inode number: %lu", (ULONG) inum);

    /*
     * Look up the group descriptor for this inode. XXX Linux inodes start at
     * one - like fortran?
     */
    grpnum = (inum - 1) / ext2fs->fs.s_inodes_per_group;
    if (ext2fs->grpnum != grpnum)
	ext2fs_group_lookup(ext2fs, grpnum);

    /*
     * Look up the inode table block for this inode.
     */
    offs = (inum - 1) - ext2fs->fs.s_inodes_per_group * grpnum;
    addr = (OFF_T) gd->bg_inode_table * ext2fs->fs_info.block_size
	+ offs * sizeof(struct ext2_inode);
    fs_read_random(&ext2fs->fs_info, (char *) dino, sizeof(*dino), addr,
		   "inode block");
    ext2fs->inum = inum;
    if (verbose)
	fprintf(logfp,
		"%lu m/l/s=%o/%d/%lu u/g=%d/%d macd=%lu/%lu/%lu/%lu\n",
		(ULONG) inum, dino->i_mode, dino->i_links_count,
		(ULONG) dino->i_size, dino->i_uid, dino->i_gid,
		(ULONG) dino->i_mtime, (ULONG) dino->i_atime,
		(ULONG) dino->i_ctime, (ULONG) dino->i_dtime);
    return (dino);
}

/* ext2fs_copy_inode - copy disk inode to generic inode */

static void ext2fs_copy_inode(struct ext2_inode * dino, FS_INODE *fs_inode)
{
    int     i;

    fs_inode->mode = dino->i_mode;
    fs_inode->nlink = dino->i_links_count;
    fs_inode->size = dino->i_size;
#ifdef i_size_high
    if (dino->i_size_high)
	fs_inode->size |= (((OFF_T) dino->i_size_high) << 32);
#endif
    fs_inode->uid = dino->i_uid;
    fs_inode->gid = dino->i_gid;
    fs_inode->mtime = dino->i_mtime;
    fs_inode->atime = dino->i_atime;
    fs_inode->ctime = dino->i_ctime;
    fs_inode->dtime = dino->i_dtime;
    if (fs_inode->direct_count != EXT2_NDIR_BLOCKS
	|| fs_inode->indir_count != EXT2_N_BLOCKS - EXT2_NDIR_BLOCKS)
	fs_inode_realloc(fs_inode, EXT2_NDIR_BLOCKS,
			 EXT2_N_BLOCKS - EXT2_NDIR_BLOCKS);
    for (i = 0; i < EXT2_NDIR_BLOCKS; i++)
	fs_inode->direct_addr[i] = dino->i_block[i];
    for ( /* void */ ; i < EXT2_N_BLOCKS; i++)
	fs_inode->indir_addr[i - EXT2_NDIR_BLOCKS] = dino->i_block[i];
}

/* ext2fs_inode_lookup - lookup inode, external interface */

static FS_INODE *ext2fs_inode_lookup(FS_INFO *fs, INUM_T inum)
{
    EXT2FS_INFO *ext2fs = (EXT2FS_INFO *) fs;
    FS_INODE *fs_inode = fs_inode_alloc(EXT2_NDIR_BLOCKS,
					EXT2_N_BLOCKS - EXT2_NDIR_BLOCKS);
    struct ext2_inode *dino = ext2fs_dinode_lookup(ext2fs, inum);

    ext2fs_copy_inode(dino, fs_inode);
    return (fs_inode);
}

/* ext2fs_inode_walk - inode iterator */

void    ext2fs_inode_walk(FS_INFO *fs, INUM_T start, INUM_T last, int flags,
			          FS_INODE_WALK_FN action, char *ptr)
{
    char   *myname = "ext2fs_inode_walk";
    EXT2FS_INFO *ext2fs = (EXT2FS_INFO *) fs;
    GRPNUM_T grpnum;
    UCHAR  *imap = 0;
    INUM_T  inum;
    INUM_T  ibase;
    struct ext2_inode *dino;
    FS_INODE *fs_inode = fs_inode_alloc(EXT2_NDIR_BLOCKS,
					EXT2_N_BLOCKS - EXT2_NDIR_BLOCKS);
    int     myflags;

    /*
     * Sanity checks.
     */
    if (start < fs->start_inum || start > fs->last_inum)
	error("%s: invalid start inode number: %lu", myname, (ULONG) start);
    if (last < fs->start_inum || last > fs->last_inum)
	error("%s: invalid last inode number: %lu", myname, (ULONG) last);

    /*
     * Iterate.
     */
    for (inum = start; inum <= last; inum++) {

	/*
	 * Be sure to use the proper group descriptor data. XXX Linux inodes
	 * start at 1, as in Fortran.
	 */
	grpnum = (inum - 1) / ext2fs->fs.s_inodes_per_group;
	if (imap == 0 || ext2fs->imap_num != grpnum) {
	    imap = ext2fs_imap_lookup(ext2fs, grpnum);
	    ibase = grpnum * ext2fs->fs.s_inodes_per_group + 1;
	}

	/*
	 * Apply the allocated/unallocated restriction.
	 */
	myflags = (isset(imap, inum - ibase) ?
		   FS_FLAG_ALLOC : FS_FLAG_UNALLOC);
	if ((flags & myflags) != myflags)
	    continue;

	/*
	 * Apply the linked/unlinked restriction.
	 */
	dino = ext2fs_dinode_lookup(ext2fs, inum);
	myflags |= (dino->i_links_count ? FS_FLAG_LINK : FS_FLAG_UNLINK);
	if ((flags & myflags) != myflags)
	    continue;

	/*
	 * Apply the used/unused restriction.
	 */
	myflags |= (dino->i_ctime ? FS_FLAG_USED : FS_FLAG_UNUSED);
	if ((flags & myflags) != myflags)
	    continue;

	/*
	 * Fill in a file system-independent inode structure and pass control
	 * to the application.
	 */
	ext2fs_copy_inode(dino, fs_inode);
	action(inum, fs_inode, myflags, ptr);
    }

    /*
     * Cleanup.
     */
    fs_inode_free(fs_inode);
}

/* ext2fs_block_walk - block iterator */

void    ext2fs_block_walk(FS_INFO *fs, DADDR_T start, DADDR_T last, int flags,
			          FS_BLOCK_WALK_FN action, char *ptr)
{
    char   *myname = "ext2fs_block_walk";
    EXT2FS_INFO *ext2fs = (EXT2FS_INFO *) fs;
    FS_BUF *fs_buf = fs_buf_alloc(fs->block_size);
    GRPNUM_T grpnum;
    UCHAR  *bmap = 0;
    DADDR_T addr;
    DADDR_T dbase;			/* first block number in group */
    DADDR_T dmin;			/* first block after inodes */
    int     myflags;

    /*
     * Sanity checks.
     */
    if (start < fs->start_block || start > fs->last_block)
	error("%s: invalid start block number: %lu", myname, (ULONG) start);
    if (last < fs->start_block || last > fs->last_block)
	error("%s: invalid last block number: %lu", myname, (ULONG) last);

    /*
     * Iterate. This is not as tricky as it could be, because the free list
     * map covers the entire disk partition, including blocks occupied by
     * group descriptor blocks, bit maps, and other non-data blocks.
     */
    for (addr = start; addr <= last; addr++) {

	/*
	 * Be sure to use the right group descriptor information. XXX There
	 * appears to be an off-by-one discrepancy between bitmap offsets and
	 * disk block numbers.
	 * 
	 * Addendum: this offset is controlled by the super block's
	 * s_first_data_block field.
	 */
#define INODE_TABLE_SIZE(ext2fs) \
	((ext2fs->fs.s_inodes_per_group * sizeof(struct ext2_inode) - 1) \
		   / ext2fs->fs_info.block_size + 1)

	grpnum = (addr - ext2fs->fs.s_first_data_block)
	    / ext2fs->fs.s_blocks_per_group;
	if (bmap == 0 || ext2fs->bmap_num != grpnum) {
	    bmap = ext2fs_bmap_lookup(ext2fs, grpnum);
	    dbase = grpnum * ext2fs->fs.s_blocks_per_group
		+ ext2fs->fs.s_first_data_block;
	    dmin = ext2fs->group.bg_inode_table + INODE_TABLE_SIZE(ext2fs);
	    if (verbose)
		fprintf(logfp, "group %d dbase %lu bmap %+ld imap %+ld inos %+ld..%ld\n",
			(int) grpnum,
			(ULONG) dbase,
			(long) ext2fs->group.bg_block_bitmap - (long) dbase,
			(long) ext2fs->group.bg_inode_bitmap - (long) dbase,
			(long) ext2fs->group.bg_inode_table - (long) dbase,
			(long) dmin - 1 - dbase);
	}

	/*
	 * Pass blocks of interest to the application. Identify meta blocks
	 * (any blocks that can't be allocated for file/directory data).
	 * 
	 * XXX With sparse superblock placement, most block groups have the
	 * block and inode bitmaps where one would otherwise find the backup
	 * superblock and the backup group descriptor blocks. The inode
	 * blocks are in the normal place, though. This leaves little gaps
	 * between the bitmaps and the inode table - and ext2fs will use
	 * those blocks for file/directory data blocks. So we must properly
	 * account for those gaps between meta blocks.
	 * 
	 * Thus, superblocks and group descriptor blocks are sometimes overlaid
	 * by bitmap blocks. This means that one can still assume that the
	 * locations of superblocks and group descriptor blocks are reserved.
	 * They just happen to be reserved for something else :-)
	 */
	myflags = (isset(bmap, addr - dbase) ?
		   FS_FLAG_ALLOC : FS_FLAG_UNALLOC);
	if ((addr >= dbase && addr < ext2fs->group.bg_block_bitmap)
	    || (addr == ext2fs->group.bg_block_bitmap)
	    || (addr == ext2fs->group.bg_inode_bitmap)
	    || (addr >= ext2fs->group.bg_inode_table && addr < dmin))
	    myflags |= FS_FLAG_META;
	if ((myflags & FS_FLAG_META) && (myflags & FS_FLAG_UNALLOC)) {
	    remark("unallocated meta block %lu!! dbase %lu dmin %lu",
		   (unsigned long) addr, (unsigned long) dbase,
		   (unsigned long) dmin);
	}
	if (flags & myflags) {
	    fs_read_block(fs, fs_buf, fs->block_size, addr, "data block");
	    action(addr, fs_buf->data, myflags, ptr);
	}
    }

    /*
     * Cleanup.
     */
    fs_buf_free(fs_buf);
}

/* ext2fs_close - close an ext2fs file system */

static void ext2fs_close(FS_INFO *fs)
{
    EXT2FS_INFO *ext2fs = (EXT2FS_INFO *) fs;

    close(ext2fs->fs_info.fd);
    free(ext2fs);
}

/* ext2fs_open - open an ext2fs file system */

FS_INFO *ext2fs_open(const char *name)
{
    char   *myname = "ext2fs_open";
    EXT2FS_INFO *ext2fs = (EXT2FS_INFO *) mymalloc(sizeof(*ext2fs));
    int     len;

    /*
     * Open the block device; linux has no raw character disk device.
     */
    if ((ext2fs->fs_info.fd = open(name, O_RDONLY)) < 0)
	error("%s: open %s: %m", myname, name);

    /*
     * Read the superblock.
     */
    len = sizeof(struct ext2_super_block);
    if (LSEEK(ext2fs->fs_info.fd, EXT2FS_SBOFF, SEEK_SET) != EXT2FS_SBOFF)
	error("%s: lseek: %m", myname);
    if (read(ext2fs->fs_info.fd, (char *) &ext2fs->fs, len) != len)
	error("%s: read superblock: %m", name);
    if (ext2fs->fs.s_magic != EXT2_SUPER_MAGIC)
	error("%s: bad magic number in superblock", name);
    if (verbose) {
#ifdef EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER
	if (ext2fs->fs.s_feature_ro_compat & EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER)
	    fprintf(logfp, "File system has sparse super blocks\n");
#endif
	fprintf(logfp, "First data block is %d\n", (int) ext2fs->fs.s_first_data_block);
    }

    /*
     * Translate some filesystem-specific information to generic form.
     */
    ext2fs->fs_info.inum_count = ext2fs->fs.s_inodes_count;
    ext2fs->fs_info.last_inum = ext2fs->fs_info.inum_count;
    ext2fs->fs_info.start_inum = 1;
    ext2fs->fs_info.block_count = ext2fs->fs.s_blocks_count;
    ext2fs->fs_info.start_block = 1;
    ext2fs->fs_info.last_block = ext2fs->fs_info.block_count - 1;
    ext2fs->fs_info.block_size =
	ext2fs->fs_info.file_bsize =
	ext2fs->fs_info.addr_bsize =
	EXT2_MIN_BLOCK_SIZE << (ext2fs->fs.s_log_block_size);
    ext2fs->fs_info.block_frags = 1;

    ext2fs->group_offset = ext2fs->fs.s_log_block_size ?
	ext2fs->fs_info.block_size : 2 * EXT2_MIN_BLOCK_SIZE;
    ext2fs->groups_count =
	(ext2fs->fs.s_blocks_count
	 - ext2fs->fs.s_first_data_block
	 + ext2fs->fs.s_blocks_per_group - 1) /
	ext2fs->fs.s_blocks_per_group;

    /*
     * Other initialization: caches, callbacks.
     */
    ext2fs->inode_map = (UCHAR *) mymalloc(ext2fs->fs_info.block_size);
    ext2fs->block_map = (UCHAR *) mymalloc(ext2fs->fs_info.block_size);
    ext2fs->fs_info.seek_pos = -1;
    ext2fs->grpnum = -1;
    ext2fs->bmap_num = -1;
    ext2fs->imap_num = -1;
    ext2fs->inum = -1;
    ext2fs->fs_info.inode_walk = ext2fs_inode_walk;
    ext2fs->fs_info.block_walk = ext2fs_block_walk;
    ext2fs->fs_info.inode_lookup = ext2fs_inode_lookup;
    ext2fs->fs_info.close = ext2fs_close;

    /*
     * Print some stats.
     */
    if (verbose)
	fprintf(logfp,
		"inodes %lu root ino %lu blocks %lu blocks/group %lu\n",
		(ULONG) ext2fs->fs.s_inodes_count,
		(ULONG) EXT2_ROOT_INO,
		(ULONG) ext2fs->fs.s_blocks_count,
		(ULONG) ext2fs->fs.s_blocks_per_group);
    return (&ext2fs->fs_info);
}

#endif

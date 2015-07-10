/*++
/* NAME
/*	fstools 3h
/* SUMMARY
/*	file system dumpster diving
/* SYNOPSIS
/*	#include "fstools.h"
/* DESCRIPTION
/* .nf

 /*
  * External interface.
  */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/fcntl.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/time.h>

 /*
  * Verbose logging.
  */
extern FILE *logfp;

 /*
  * Solaris 2.x. Build for large files when dealing with filesystems > 2GB.
  * With the 32-bit file model, needs pread() to access filesystems > 2GB.
  */
#if defined(SUNOS5)
#define SUPPORTED
#define HAVE_UFS_FFS
#include <sys/vnode.h>
#include <sys/fs/ufs_inode.h>
#include <sys/fs/ufs_fs.h>
#include <sys/fs/ufs_fsdir.h>
#include <sys/sysmacros.h>
#define LSEEK		lseek
#define OFF_T		off_t
#define ROOTINO		UFSROOTINO
#define INOTIME(t)	(t)
#define STRTOUL		strtoul
#define DADDR_T		daddr_t
#define UFS_TYPE	"ufs"
#define DEF_FSTYPE	UFS_TYPE
#define INO_TO_CG	itog
#endif

 /*
  * SunOS 4.x cannot handle filesystems > 2GB.
  */
#if defined(SUNOS4)
#define SUPPORTED
#define HAVE_UFS_FFS
#include <sys/vnode.h>
#include <ufs/inode.h>
#include <ufs/fs.h>
#include <ufs/fsdir.h>
#define LSEEK		lseek
#define OFF_T		off_t
#define STRTOUL		strtol
#define INOTIME(t)	(t)
#define DADDR_T		daddr_t
#define UFS_TYPE	"4.2"
#define DEF_FSTYPE	UFS_TYPE
#define INO_TO_CG	itog

extern char *optarg;
extern int optind;

#endif

 /*
  * FreeBSD can handle filesystems > 2GB.
  */
#if defined(FREEBSD2)
#define SUPPORTED
#define HAVE_UFS_FFS
#include <sys/vnode.h>
#include <ufs/ufs/quota.h>
#include <ufs/ufs/inode.h>
#include <ufs/ffs/fs.h>
#define LSEEK		lseek
#define OFF_T		off_t
#define STRTOUL		strtoul
#define itod(fs,i)	ino_to_fsba(fs,i)
#define itoo(fs,i)	ino_to_fsbo(fs,i)
#define INOTIME(t)	(t).tv_sec
#define DADDR_T		daddr_t
#define UFS_TYPE	"ufs"
#define DEF_FSTYPE	UFS_TYPE
#define INO_TO_CG	ino_to_cg
#endif

#if defined(FREEBSD3) || defined(FREEBSD4)
#define SUPPORTED
#define HAVE_UFS_FFS
#include <sys/vnode.h>
#include <ufs/ufs/quota.h>
#include <ufs/ufs/inode.h>
#include <ufs/ffs/fs.h>
#define LSEEK		lseek
#define OFF_T		off_t
#define STRTOUL		strtoul
#define itod(fs,i)	ino_to_fsba(fs,i)
#define itoo(fs,i)	ino_to_fsbo(fs,i)
#define INOTIME(t)	(t)
#define DADDR_T		daddr_t
#define UFS_TYPE	"ufs"
#define DEF_FSTYPE	UFS_TYPE
#define INO_TO_CG	ino_to_cg
#endif

#if defined(FREEBSD5) || defined(FREEBSD6) || defined(FREEBSD7)
#define SUPPORTED
#include <sys/vnode.h>
#include <ufs/ufs/quota.h>
#include <ufs/ufs/inode.h>
#include <ufs/ffs/fs.h>
#define LSEEK		lseek
#define OFF_T		off_t
#define STRTOUL		strtoul
#define itod(fs,i)	ino_to_fsba(fs,i)
#define itoo(fs,i)	ino_to_fsbo(fs,i)
#define INOTIME(t)	(t)
#define DADDR_T		int64_t
#define UFS_TYPE	"ufs"
#define DEF_FSTYPE	UFS_TYPE
#define INO_TO_CG	ino_to_cg
#endif

 /*
  * BSD/OS can handle filesystems > 2GB.
  */
#if defined(BSDI2)
#define SUPPORTED
#define HAVE_UFS_FFS
#include <sys/vnode.h>
#include <ufs/ufs/quota.h>
#include <ufs/ufs/inode.h>
#include <ufs/ffs/fs.h>
#define LSEEK		lseek
#define OFF_T		off_t
#define STRTOUL		strtoul
#define itod(fs,i)	ino_to_fsba(fs,i)
#define itoo(fs,i)	ino_to_fsbo(fs,i)
#define INOTIME(t)	(t).ts_sec
#define DADDR_T		daddr_t
#define UFS_TYPE	"ufs"
#define DEF_FSTYPE	UFS_TYPE
#define INO_TO_CG	ino_to_cg
#endif

#if defined(BSDI3) || defined(BSDI4)
#define SUPPORTED
#define HAVE_UFS_FFS
#include <sys/vnode.h>
#include <ufs/ufs/quota.h>
#include <ufs/ufs/inode.h>
#include <ufs/ffs/fs.h>
#define LSEEK		lseek
#define OFF_T		off_t
#define STRTOUL		strtoul
#define itod(fs,i)	ino_to_fsba(fs,i)
#define itoo(fs,i)	ino_to_fsbo(fs,i)
#define INOTIME(t)	(t).tv_sec
#define DADDR_T		daddr_t
#define UFS_TYPE	"ufs"
#define DEF_FSTYPE	UFS_TYPE
#define INO_TO_CG	ino_to_cg
#endif

 /*
  * OpenBSD2 looks like BSD/OS 3.x.
  */
#if defined(OPENBSD2) || defined(OPENBSD3) || defined(OPENBSD4)
#define SUPPORTED
#define HAVE_UFS_FFS
#include <sys/vnode.h>
#include <ufs/ufs/quota.h>
#include <ufs/ufs/inode.h>
#include <ufs/ffs/fs.h>
#define LSEEK		lseek
#define OFF_T		off_t
#define STRTOUL		strtoul
#define itod(fs,i)	ino_to_fsba(fs,i)
#define itoo(fs,i)	ino_to_fsbo(fs,i)
#define INOTIME(t)	(t)
#define DADDR_T		daddr_t
#define UFS_TYPE	"ufs"
#define DEF_FSTYPE	UFS_TYPE
#define INO_TO_CG	ino_to_cg
#endif

 /*
  * Linux 2.whatever. We'll see how stable the interfaces are.
  */
#if defined(LINUX2)
#define SUPPORTED
#include <linux/ext2_fs.h>
#define HAVE_EXT2FS
#define HAVE_DTIME
#if (_FILE_OFFSET_BITS == 64)
#define LSEEK		lseek
#define OFF_T		off_t
#else
#define USE_MYLSEEK
#define HAVE_LLSEEK
#define LSEEK		mylseek
#define OFF_T		long long
#endif
#define STRTOUL		strtoul
#define DADDR_T		__u32
#define EXT2FS_TYPE	"ext2fs"
#define DEF_FSTYPE	EXT2FS_TYPE
#define EXT2FS_SBOFF	1024		/* no symbolic constant? */
#ifndef NBBY				/* NIH */
#define NBBY 8
#endif
#ifndef isset				/* NIH */
#define isset(a,i)	((a)[(i)/NBBY] & (1<<((i)%NBBY)))
#endif
#endif

 /*
  * Catch-all.
  */
#ifndef SUPPORTED
#error "This operating system is not supported"
#endif

typedef unsigned long INUM_T;
typedef unsigned long ULONG;
typedef unsigned long CGNUM_T;
typedef unsigned long GRPNUM_T;
typedef unsigned char UCHAR;

typedef struct FS_INFO FS_INFO;
typedef struct FS_BUF FS_BUF;
typedef struct FS_INODE FS_INODE;
typedef void (*FS_INODE_WALK_FN) (INUM_T, FS_INODE *, int, char *);
typedef void (*FS_BLOCK_WALK_FN) (DADDR_T, char *, int, char *);

struct FS_INFO {
    int     fd;				/* open raw device */
    INUM_T  inum_count;			/* number of inodes */
    INUM_T  start_inum;			/* first inode */
#define root_inum start_inum		/* tctutils compatibility */
    INUM_T  last_inum;			/* LINUX starts at 1 */
    DADDR_T block_count;		/* number of blocks */
    DADDR_T start_block;		/* in case start at 1 */
    DADDR_T last_block;			/* in case start at 1 */
    int     block_size;			/* block size in bytes */
    int     file_bsize;			/* file block size in bytes */
    int     addr_bsize;			/* indirect block size in bytes */
    int     block_frags;		/* blocks in logical block */
    OFF_T   seek_pos;			/* current seek position */
    void    (*inode_walk) (FS_INFO *, INUM_T, INUM_T, int, FS_INODE_WALK_FN, char *);
    void    (*block_walk) (FS_INFO *, DADDR_T, DADDR_T, int, FS_BLOCK_WALK_FN, char *);
    FS_INODE *(*inode_lookup) (FS_INFO *, INUM_T);
    void    (*close) (FS_INFO *);
};

#define FS_FLAG_LINK	(1<<0)		/* link count > 0 */
#define FS_FLAG_UNLINK	(1<<1)		/* link count == 0 */
#define FS_FLAG_ALLOC	(1<<2)		/* allocated */
#define FS_FLAG_UNALLOC	(1<<3)		/* unallocated */
#define FS_FLAG_USED	(1<<4)		/* used */
#define FS_FLAG_UNUSED	(1<<5)		/* never used */
#define FS_FLAG_META	(1<<6)		/* meta data block */
#define FS_FLAG_ALIGN	(1<<7)		/* block align */
#define FS_FLAG_TMHDR	(1<<8)		/* show tm header */

 /*
  * I/O buffer, used for all forms of I/O.
  */
struct FS_BUF {
    char   *data;			/* buffer memory */
    int     size;			/* buffer size */
    int     used;			/* amount of space used */
    DADDR_T addr;			/* start block */
};

extern FS_BUF *fs_buf_alloc(int);
extern void fs_buf_free(FS_BUF *);

 /*
  * Generic inode structure for filesystem-independent operations.
  */
struct FS_INODE {
    mode_t  mode;			/* type and permission */
    int     nlink;			/* link count */
    OFF_T   size;			/* file size */
    uid_t   uid;			/* owner */
    gid_t   gid;			/* group */
    time_t  mtime;			/* last modified */
    time_t  atime;			/* last access */
    time_t  ctime;			/* last status change */
#ifdef HAVE_DTIME
    time_t  dtime;			/* delete time */
#endif
    DADDR_T *direct_addr;		/* direct blocks */
    int     direct_count;		/* number of blocks */
    DADDR_T *indir_addr;		/* indirect blocks */
    int     indir_count;		/* number of blocks */
};

extern FS_INODE *fs_inode_alloc(int, int);
extern FS_INODE *fs_inode_realloc(FS_INODE *, int, int);
extern void fs_inode_free(FS_INODE *);

 /*
  * Generic routines.
  */
extern FS_INFO *fs_open(const char *, const char *);
extern void fs_read_block(FS_INFO *, FS_BUF *, int, DADDR_T, const char *);
extern void fs_read_random(FS_INFO *, char *, int, OFF_T, const char *);
extern void fs_copy_file(FS_INFO *, INUM_T, int);

 /*
  * Support for BSD FFS and lookalikes.
  */
extern FS_INFO *ffs_open(const char *);

 /*
  * Support for LINUX ext2fs.
  */
extern FS_INFO *ext2fs_open(const char *);

 /*
  * Support for long seeks.
  */
extern OFF_T mylseek(int, OFF_T, int);

/* LICENSE
/* .ad
/* .fi
/*	This software is distributed under the IBM Public License.
/* AUTHOR(S)
/*	Wietse Venema
/*	IBM T.J. Watson Research
/*	P.O. Box 704
/*	Yorktown Heights, NY 10598, USA
/*--*/

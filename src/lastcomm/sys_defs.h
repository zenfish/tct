 /*
  * lastcomm porting file.
  * 
  * Author: Wietse Venema, IBM T.J. Watson Research Center, Hawthorne, USA.
  * 
  * This software is distributed under the IBM Public License.
  */
#if defined(FREEBSD2) || defined(FREEBSD3)  || defined(FREEBSD4) \
	|| defined(FREEBSD5) || defined(FREEBSD6)  || defined(FREEBSD7) \
	|| defined(BSDI2) || defined(BSDI3) || defined(BSDI4) \
	|| defined(OPENBSD2) || defined(OPENBSD3) || defined(OPENBSD4)
#define SUPPORTED
#define _PATH_ACCT	"/var/account/acct"
#define HAVE_COMP_BLOCK_IO_COUNT
#define HAVE_MEMORY_USAGE
#endif

#ifdef FREEBSD7
#define HAVE_ACCTV2
#endif

#ifdef SUNOS5_0_5
#define SUPPORTED
#define _PATH_ACCT	"/var/adm/pacct"
#define USE_SYSMACROS_H
#define SUN_SRC_COMPAT
#define AHZ		100
#define PRINT_LINESIZE	8
#define PRINT_NAMESIZE	8
#define HAVE_COMP_BLOCK_RW_COUNT
#define HAVE_COMP_CHAR_IO_COUNT
#define HAVE_COMP_MEMORY_USAGE
#endif

#ifdef SUNOS5
#define SUPPORTED
#define _PATH_ACCT	"/var/adm/pacct"
#define USE_SYSMACROS_H
#define SUN_SRC_COMPAT
#define AHZ		100
#define PRINT_LINESIZE	8
#define PRINT_NAMESIZE	8
#define HAVE_COMP_BLOCK_RW_COUNT
#define HAVE_COMP_CHAR_IO_COUNT
#define HAVE_COMP_MEMORY_USAGE
#endif

#ifdef SUNOS4
#define SUPPORTED
#define _PATH_ACCT	"/var/adm/pacct"
#define AHZ		64
#define PRINT_LINESIZE	8
#define PRINT_NAMESIZE	8
#define HAVE_COMP_BLOCK_RW_COUNT
#define HAVE_COMP_CHAR_IO_COUNT
#define HAVE_COMP_MEMORY_USAGE
extern char *optarg;
extern int optind;
extern int getopt();

#endif

#ifdef LINUX2_0_1
#define SUPPORTED
#define _PATH_ACCT	"/var/log/pacct"
#define NODEV		0xffff
#include <sys/sysmacros.h>
#ifndef major
#define major(x)	((x)>>8)
#define minor(x)	((x)&0xff)
#endif
#define HAVE_MAJOR_PFLTS
#define HAVE_EXIT_STATUS
#define PRINT_LINESIZE	8
#define PRINT_NAMESIZE	8
#endif

#ifdef LINUX2
#define SUPPORTED
#define _PATH_ACCT	"/var/log/pacct"
#define NODEV		0xffff
#define HAVE_COMP_MAJOR_PFLTS
#define HAVE_COMP_EXIT_STATUS
#define HAVE_COMP_MEMORY_USAGE
#define HAVE_COMP_CHAR_IO_COUNT
#define HAVE_COMP_BLOCK_RW_COUNT
#define HAVE_COMP_SWAP_USAGE
#define HAVE_EXIT_STATUS
#define PRINT_LINESIZE	8
#define PRINT_NAMESIZE	8
#endif

#ifndef SUPPORTED
#error "This system is not supported"
#endif

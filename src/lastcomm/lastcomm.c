/*
 * Copyright (c) 1980, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *  $Id: lastcomm.c,v 1.4 1996/06/30 11:58:20 wosch Exp $
 */

#ifndef lint
static char copyright[] =
"@(#) Copyright (c) 1980, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)lastcomm.c	8.1 (Berkeley) 6/6/93";
#endif /* not lint */

#include "sys_defs.h"

#ifdef USE_SYSMACROS_H
#include <sys/sysmacros.h>
#endif

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/acct.h>

#include <time.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <struct.h>
#include <unistd.h>
#include <utmp.h>
/*#include "pathnames.h"*/

#ifdef HAVE_ACCTV2
#define ACCT_STRUCT	acctv2
#define ACCT_FLAG	ac_flagx
#define expand(x)	(x)
#define AHZ		1000000
#else
#define ACCT_STRUCT	acct
#define ACCT_FLAG	ac_flag
time_t	 expand(u_int);
#endif
char	*flagbits(int);
char	*getdev(dev_t);
int	 requested(char *[], struct ACCT_STRUCT *);
void	 usage(void);
char	*user_from_uid();

char	*print_uid(uid_t);
#define user_from_uid(u,f) print_uid(u)
#define err(e, fmt, s) { perror(s); exit(e); }

#define AC_UTIME 1 /* user */
#define AC_STIME 2 /* system */
#define AC_ETIME 4 /* elapsed */
#define AC_CTIME 8 /* user + system time, default */

#define AC_BTIME 16 /* starting time */
#define AC_FTIME 32 /* exit time (starting time + elapsed time )*/

#define AC_HZ ((double)AHZ)

#ifndef PRINT_NAMESIZE
#define PRINT_NAMESIZE	UT_NAMESIZE
#define PRINT_LINESIZE	UT_LINESIZE
#endif

int
main(argc, argv)
	int argc;
	char *argv[];
{
	register char *p;
	struct ACCT_STRUCT ab;
	struct stat sb;
	FILE *fp;
	off_t size;
	time_t t;
	int ch;
	char *acctfile;
	int time = 0;
	int time_machine = 0;

	acctfile = _PATH_ACCT;
	while ((ch = getopt(argc, argv, "f:tusecSE")) != EOF)
		switch((char)ch) {
		case 'f':
			acctfile = optarg;
			break;

		case 't':
			time_machine = 1;
			break;
		case 'u': 
			time |= AC_UTIME; /* user time */
			break;
		case 's':
			time |= AC_STIME; /* system time */
			break;
		case 'e':
			time |= AC_ETIME; /* elapsed time */
			break;
		case 'c':
			time |= AC_CTIME; /* user + system time */
			break;

		case 'S':
			time |= AC_BTIME; /* starting time */
			break;
		case 'E':
			/* exit time (starting time + elapsed time )*/
			time |= AC_FTIME; 
			break;

		case '?':
		default:
			usage();
		}

	/* default user + system time and starting time */
	if (!time) {
	    time = AC_CTIME | AC_BTIME;
	}

	argc -= optind;
	argv += optind;

	/* Open the file. */
	if ((fp = fopen(acctfile, "r")) == NULL || fstat(fileno(fp), &sb))
		err(1, "%s", acctfile);

	/*
	 * Round off to integral number of accounting records, probably
	 * not necessary, but it doesn't hurt.
	 */
	size = sb.st_size - sb.st_size % sizeof(struct ACCT_STRUCT);

	/* Print the optional time machine header. */
	if (time_machine) {
		char myhostname[BUFSIZ];
		time_t time();
		time_t now;
		if (gethostname(myhostname, sizeof(myhostname) - 1) < 0)
			err(1, "%s", "gethostname");
		time(&now);
		printf("class|host|start_time\n%s|%s|%lu\n",
			"lastcomm", myhostname, (unsigned long) now);
		printf("command|flags|uid|gid|tty");
		printf("|user_time|system_time|start_time|elapsed_time");
#if defined(HAVE_MEMORY_USAGE) || defined(HAVE_COMP_MEMORY_USAGE)
		printf("|memory_usage");
#endif
#ifdef HAVE_COMP_CHAR_IO_COUNT
		printf("|character_io");
#endif
#ifdef HAVE_COMP_BLOCK_IO_COUNT
		printf("|block_io");
#endif
#ifdef HAVE_COMP_BLOCK_RW_COUNT
		printf("|block_io");
#endif
#if defined(HAVE_MAJOR_PFLTS) || defined(HAVE_COMP_MAJOR_PFLTS)
		printf("|major_page_faults|minor_page_faults");
#endif
#ifdef HAVE_EXIT_STATUS
		printf("|exit_status");
#endif
#ifdef HAVE_COMP_SWAP_USAGE
		printf("|swaps");
#endif
		printf("\n");
	}

	/* Check if any records to display. */
	if (size < sizeof(struct ACCT_STRUCT))
		exit(0);

	/*
	 * Seek to before the last entry in the file; use lseek(2) in case
	 * the file is bigger than a "long".
	 */
	size -= sizeof(struct ACCT_STRUCT);
	if (lseek(fileno(fp), size, SEEK_SET) == -1)
		err(1, "%s", acctfile);

	for (;;) {
		if (size < 0)
			break;

		if (fread(&ab, sizeof(struct ACCT_STRUCT), 1, fp) != 1)
			err(1, "%s", acctfile);

		size -= sizeof(struct ACCT_STRUCT);

		if (size >= 0)
		if (fseek(fp, 2 * -(long)sizeof(struct ACCT_STRUCT), SEEK_CUR) == -1)
			err(1, "%s", acctfile);

		if (ab.ac_comm[0] == '\0') {
			ab.ac_comm[0] = '?';
			ab.ac_comm[1] = '\0';
		} 
		if (*argv && !requested(argv, &ab))
			continue;

		/* Optionally produce time machine format output */
#define UNSIG(x) ((unsigned char *)(x))
		if (time_machine) {
			for (p = ab.ac_comm; *p != 0
				&& p < ab.ac_comm + fldsiz(ACCT_STRUCT, ac_comm); p++)
				if (isascii(*UNSIG(p)) && isprint(*UNSIG(p))
				&& *UNSIG(p) != '|' && *UNSIG(p) != '%')
					putchar(*UNSIG(p));
				else
					printf("%%%02X", *UNSIG(p));
			printf("|%s|%lu|%lu",
				flagbits(ab.ACCT_FLAG),
				(unsigned long) ab.ac_uid,
				(unsigned long) ab.ac_gid);
			printf("|%s", getdev(ab.ac_tty));

			printf("|%.3f|%.3f|%ld|%.3f",
				expand(ab.ac_utime) / AC_HZ,
				expand(ab.ac_stime) / AC_HZ,
				(long) ab.ac_btime,
				expand(ab.ac_etime) / AC_HZ);

#ifdef HAVE_MEMORY_USAGE
			printf("|%lu", (unsigned long) ab.ac_mem);
#endif
#ifdef HAVE_COMP_MEMORY_USAGE
			printf("|%lu", (unsigned long) expand(ab.ac_mem));
#endif
#ifdef HAVE_COMP_CHAR_IO_COUNT
			printf("|%lu", (unsigned long) expand(ab.ac_io));
#endif
#ifdef HAVE_COMP_BLOCK_IO_COUNT
			printf("|%lu", (unsigned long) expand(ab.ac_io));
#endif
#ifdef HAVE_COMP_BLOCK_RW_COUNT
			printf("|%lu", (unsigned long) expand(ab.ac_rw));
#endif
#ifdef HAVE_MAJOR_PFLTS
			printf("|%lu|%lu",
				(unsigned long) ab.ac_majflt,
				(unsigned long) ab.ac_minflt);
#endif
#ifdef HAVE_COMP_MAJOR_PFLTS
			printf("|%lu|%lu",
				(unsigned long) expand(ab.ac_majflt),
				(unsigned long) expand(ab.ac_minflt));
#endif
#ifdef HAVE_EXIT_STATUS
			printf("|%ld", (long) ab.ac_exitcode);
#endif
#ifdef HAVE_COMP_SWAP_USAGE
			printf("|%lu", (unsigned long) expand(ab.ac_swaps));
#endif
			printf("\n");
			continue;
		}

		/* Produce the default output format. */
		for (p = &ab.ac_comm[0];
		    p < &ab.ac_comm[fldsiz(ACCT_STRUCT, ac_comm)] && *p; ++p)
			if (!isprint(*p))
				*p = '?';
		(void)printf("%-*.*s %-7s %-*s %-*s ",
			     fldsiz(ACCT_STRUCT, ac_comm),
			     fldsiz(ACCT_STRUCT, ac_comm),
			     ab.ac_comm,
			     flagbits(ab.ACCT_FLAG),
			     PRINT_NAMESIZE, user_from_uid(ab.ac_uid, 0),
			     PRINT_LINESIZE, getdev(ab.ac_tty));
		
		
		/* user + system time */
		if (time & AC_CTIME) {
			(void)printf("%6.3f secs ", 
				     (expand(ab.ac_utime) + 
				      expand(ab.ac_stime))/AC_HZ);
		}
		
		/* usr time */
		if (time & AC_UTIME) {
			(void)printf("%6.3f us ", expand(ab.ac_utime)/AC_HZ);
		}
		
		/* system time */
		if (time & AC_STIME) {
			(void)printf("%6.3f sy ", expand(ab.ac_stime)/AC_HZ);
		}
		
		/* elapsed time */
		if (time & AC_ETIME) {
			(void)printf("%8.3f es ", expand(ab.ac_etime)/AC_HZ);
		}
		
		/* starting time */
		if (time & AC_BTIME) {
			(void)printf("%.16s ", ctime((time_t *)&ab.ac_btime));
		}
		
		/* exit time (starting time + elapsed time )*/
		if (time & AC_FTIME) {
			t = ab.ac_btime;
			t += (time_t)(expand(ab.ac_etime)/AC_HZ);
			(void)printf("%.16s ", 
				     ctime(&t));
		}
		printf("\n");
 	}
 	exit(0);
}

#ifndef HAVE_ACCTV2

time_t
expand(t)
	u_int t;
{
	register time_t nt;

	nt = t & 017777;
	t >>= 13;
	while (t) {
		t--;
		nt <<= 3;
	}
	return (nt);
}

#endif

char *
flagbits(f)
	register int f;
{
	static char flags[20] = "-";
	char *p;

#define	BIT(flag, ch)	if (f & flag) *p++ = ch

	p = flags + 1;
	BIT(ASU, 'S');
	BIT(AFORK, 'F');
#ifdef ACOMPAT
	BIT(ACOMPAT, 'C');
#endif
#ifdef ACORE
	BIT(ACORE, 'D');
#endif
#ifdef AXSIG
	BIT(AXSIG, 'X');
#endif
	*p = '\0';
	return (flags);
}

int
requested(argv, acp)
	register char *argv[];
	register struct ACCT_STRUCT *acp;
{
	register char *p;

	do {
		p = user_from_uid(acp->ac_uid, 0);
		if (!strcmp(p, *argv))
			return (1);
		if ((p = getdev(acp->ac_tty)) && !strcmp(p, *argv))
			return (1);
		if (!strncmp(acp->ac_comm, *argv, fldsiz(ACCT_STRUCT, ac_comm)))
			return (1);
	} while (*++argv);
	return (0);
}

char *
getdev(dev)
	dev_t dev;
{
	static dev_t lastdev = (dev_t)-1;
	static char lastname[BUFSIZ];
	if (dev == NODEV)			/* Special case. */
		return ("__");
	if (dev == lastdev)			/* One-element cache. */
		return (lastname);
	lastdev = dev;
	sprintf(lastname, "%d,%d", (int) major(dev), (int) minor(dev));
	return (lastname);
}

void
usage()
{
	(void)fprintf(stderr,
	    "lastcomm [-EScestu] [ -f file ] [command ...] [user ...] [tty ...]\n");
	exit(1);
}

char *
print_uid(uid_t uid)
{
	static char buf[BUFSIZ];

	sprintf(buf, "%lu", (unsigned long) uid);
	return (buf);
}

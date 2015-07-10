/*++
/* NAME
/*	findkey 1
/* SUMMARY
/*	find high-entropy data
/* SYNOPSIS
/* .ad
/* .fi
/*	\fBfindkey\fR [\fB-cdlmx\fR] [\fB-b \fIbias\fR] [\fB-w \fIwindow\fR]
/*	[\fB-t \fIthreshold\fR] \fIfile...\fR
/* DESCRIPTION
/*	\fBfindkey\fR moves a fixed-size window over the named files (stdin
/*	by default) and reports window positions containing a large number
/*	of different symbols. This approach, described by Shamir and van
/*	Someren, simplifies Shannon's entropy formula and allows for faster
/*	searching.
/*
/*	By default, overlapping regions of \fIwindow\fR bytes with 
/*	high-entropy content are reported as one large region:
/*
/* .ti +4
/*	\fIfile start [content] length\fR
/*
/*	Fields are tab separated. By default no content is shown.
/*
/*	In order to display the content of potential keys, specify the
/*	\fB-x\fR option (show content).
/*
/*	In order to report potential keys individually, specify the 
/*	\fB-d\fR option (detail mode).
/*
/*	 Options:
/* .IP \fB-c\fR
/*	Calibration mode. Report each high-entropy region of \fIwindow\fR 
/*	bytes as: \fIfile start threshold bias [content]\fR. This is useful
/*	for deriving good \fIthreshold\fR and \fIbias\fR values from real
/*	data. Use of this option implies \fB-d\fR.
/* .IP \fB-d\fR
/*	Detail mode. Report each high-entropy region of \fIwindow\fR bytes
/*	as: \fIfile start [content]\fR. This option increases the amount
/*	of output considerably. Combine this option with with \fB-x\fR to 
/*	display the content of individual keys.
/* .IP "\fB-b \fIbias\fR (default: 31)"
/*	Exclude data that is too monotonic. \fBfindkey\fR counts how often
/*	successive byte values increase or decrease. The number of increments
/*	and decrements in a window must not differ by more than \fIbias\fR.
/*	For 64-byte windows, 22 is a good bias value.
/* .IP \fB-l\fR
/*	Report only the names of files containing high-entropy regions.
/* .IP "\fB-m\fR (default)"
/*	Merge mode. Report multiple overlapping high-entropy windows as
/*	one region, using the format: \fIfile start [content] length\fR.
/*	Use of this option can reduce the amount of output considerably.
/* .IP "\fB-t \fIthreshold\fR (default: 95)"
/*	Search for windows with at least \fIthreshold\fR different symbols.
/*	For 64-byte windows, 52 is a good threshold value.
/* .IP "\fB-w \fIwindow\fR (default: 128)"
/*	Search for high-entropy regions of at least \fIwindow\fR bytes.
/*	Use of \fB-w\fR also requires use of the \fB-b\fR and \fB-t\fR options.
/* .IP \fB-x\fR
/*	Display high-entropy content in hexadecimal form.
/* HISTORY
/* .fi
/* .ad
/*	This \fBfindkey\fR command was written for the coroner's toolkit.
/* LICENSE
/*	This software is distributed under the IBM Public License.
/* SEE ALSO
/* .ad
/* .fi
/*	A. Shamir and N. van Someren, Playing Hide and Seek With Stored
/*	Keys, 1998.
/* .nf
/* .na
/*	http://www.ncipher.com/products/files/papers/anguilla/keyhide2.pdf.
/* .PP
/* .ad
/* .fi
/*      C.E.  Shannon, A mathematical theory of communication. Bell
/*	System Technical Journal 27 (1948) 379-423, 632-656.
/* .nf
/* .na
/*	http://cm.bell-labs.com/cm/ms/what/shannonday/shannon1948.ps.gz
/* AUTHOR(S)
/*	Wietse Venema
/*	IBM T.J. Watson Research
/*	P.O. Box 704
/*	Yorktown Heights, NY 10598, USA
/*--*/

/* System library. */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

/* SunOS4 needs this - I hope it does not break on some pedantic systems. */

extern char *optarg;
extern int optind;

/* Utility library. */

#include <error.h>

/* Application-specific. */

#define ALL_BYTES	256
#define DEF_WINDOW_SIZE	128		/* 64 for exportable keys */
#define DEF_THRESHOLD	95		/* 52 for exportable keys */
#define DEF_BIAS	31		/* 22 for exportable keys */

#define FLAG_DETAIL	(1<<0)		/* report individual windows */
#define FLAG_LIST	(1<<1)		/* list file names only */
#define FLAG_HEXDUMP	(1<<2)		/* list content */
#define FLAG_CALIBR	(1<<3)		/* calibration mode */

/* search_he - search stream for high-entry sequence */

void    search_he(char *path, FILE *fp, int window_size, int threshold, int bias, int flags)
{
    unsigned char ring_buf[window_size];
    int     byte_frequency[ALL_BYTES];
    int     distinct_bytes = 0;
    unsigned file_offset = 0;
    int     ring_offset;
    int     ch;
    int     i;
    int     high_entropy = 0;
    unsigned start;
    unsigned end;
    int     last = 0;
    int     direction = 0;
    int     dir_buf[window_size];

    /*
     * Initialize.
     */
    for (i = 0; i < ALL_BYTES; i++)
	byte_frequency[i] = 0;
    for (i = 0; i < window_size; i++)
	dir_buf[i] = 0;

    /*
     * A short and sweet loop that slides the window over the file and that
     * on the fly maintains symbol frequency counts and distinct symbol
     * counts.
     */
    while ((ch = getc(fp)) != EOF) {
	ring_offset = file_offset % window_size;
	if (file_offset >= window_size)
	    if (--byte_frequency[ring_buf[ring_offset]] == 0)
		distinct_bytes -= 1;
	ring_buf[ring_offset] = ch;
	if (byte_frequency[ch]++ == 0)
	    distinct_bytes += 1;
	file_offset++;
	direction -= dir_buf[ring_offset];
	direction += dir_buf[ring_offset] =
	    (ch < last ? -1 : ch == last ? 0 : +1);
	last = ch;
	if (file_offset < window_size)
	    continue;

	/*
	 * A lot of code to report what we found.
	 */
	if (distinct_bytes >= threshold
	    && direction <= bias
	    && direction >= -bias) {

	    /*
	     * Report each high-entropy window by itself. This can produce an
	     * incredible amount of output, but is useful for tuning and for
	     * reporting individual keys.
	     */
	    if (flags & FLAG_DETAIL) {
		printf("%s\t%u", path, file_offset - window_size);
		if (flags & FLAG_CALIBR)
		    printf("\t%u\t%d", distinct_bytes,
			   direction < 0 ? -direction : direction);
		if (flags & FLAG_HEXDUMP) {
		    printf("\t");
		    for (i = 0; i < window_size; i++)
			printf("%02x", ring_buf[(file_offset + i) % window_size]);
		}
		printf("\n");
		continue;
	    }

	    /*
	     * Report file names only.
	     */
	    if ((flags & FLAG_LIST) != 0) {
		printf("%s\n", path);
		break;
	    }

	    /*
	     * Concatenate overlapping high-entropy windows. This reduces the
	     * amount of output considerably.
	     */
	    if (high_entropy == 0) {
		high_entropy = 1;
		start = file_offset - window_size;
		printf("%s\t%u\t", path, start);
		if (flags & FLAG_HEXDUMP)
		    for (i = 0; i < window_size; i++)
			printf("%02x", ring_buf[(file_offset + i) % window_size]);
	    } else if (flags & FLAG_HEXDUMP)
		printf("%02x", ch);
	    end = file_offset;

	} else {

	    if (flags & FLAG_DETAIL)
		continue;

	    /*
	     * Concatenate overlapping high-entropy windows. This reduces the
	     * amount of output considerably.
	     */
	    if (high_entropy != 0 && file_offset - window_size >= end) {
		high_entropy = 0;
		if (flags & FLAG_HEXDUMP)
		    printf("\t");
		printf("%u\n", end - start);
	    }
	}
    }
    if (high_entropy != 0) {
	if (flags & FLAG_HEXDUMP)
	    printf("\t");
	printf("%u\n", file_offset - start);
    }
}

/* main - driver */

int     main(int argc, char **argv)
{
    FILE   *fp;
    int     window_size = DEF_WINDOW_SIZE;
    int     threshold = DEF_THRESHOLD;
    int     bias = DEF_BIAS;
    int     flags = 0;
    int     ch;
    int     i;

    progname = argv[0];

    /*
     * Parse JCL.
     */
    while ((ch = getopt(argc, argv, "b:cdlmt:w:x")) > 0) {
	switch (ch) {
	case 'c':
	    flags |= (FLAG_DETAIL | FLAG_CALIBR);
	    break;
	case 'd':
	    flags |= FLAG_DETAIL;
	    break;
	case 'b':
	    if ((bias = atoi(optarg)) <= 0)
		error("bad bias value: %s", optarg);
	    break;
	case 'l':
	    flags |= FLAG_LIST;
	    break;
	case 'm':
	    flags &= ~(FLAG_DETAIL | FLAG_CALIBR);
	    break;
	case 't':
	    if ((threshold = atoi(optarg)) <= 0)
		error("bad threshold value: %s", optarg);
	    if (threshold > ALL_BYTES)
		error("threshold cannot be larger than %d", ALL_BYTES);
	    break;
	case 'w':
	    if ((window_size = atoi(optarg)) <= 0)
		error("bad window size: %s", optarg);
	    break;
	case 'x':
	    flags |= FLAG_HEXDUMP;
	    break;
	default:
	    error("usage: %s [-b bias] [-c (calibrate)] [-d (detail)] [-l (names only)] [-m (merge windows)] [-t threshold] [-w window_size] [-x (hexdump)] [file...]",
		  progname);
	}
    }

    /*
     * Sanity checks.
     */
    if (threshold > window_size)
	error("threshold %d exceeds window size %d", threshold, window_size);
    if (bias > window_size)
	remark("bias cannot exceed window_size");

    /*
     * Search for keys.
     */
    if (argc == optind) {
	search_he("(stdin)", stdin, window_size, threshold, bias, flags);
    } else {
	for (i = optind; i < argc; i++) {
	    if ((fp = fopen(argv[i], "r")) == 0) {
		remark("open %s: %m", argv[i]);
	    } else {
		search_he(argv[i], fp, window_size, threshold, bias, flags);
		if (fclose(fp) != 0)
		    remark("read %s: %m\n", argv[i]);
	    }
	}
    }
    exit(0);
}

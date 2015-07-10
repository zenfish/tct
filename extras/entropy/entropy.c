/*++
/* NAME
/*	entropy 1
/* SUMMARY
/*	compute data entropy
/* SYNOPSIS
/* .ad
/* .fi
/*	\fBentropy\fR [\fB-b \fIblock_size\fR] \fIfile...\fR
/* DESCRIPTION
/*	\fBentropy\fR reports entropy values for the named files (standard
/*	input by default). The entropy is computed with Shannon's formula
/*	applied to byte streams. The result gives the average number of bits
/*	of information per byte, under the assumption that the input has no
/*	specific structure.
/*
/*	For each file, entropy is reported as follows:
/*
/* .ti +4
/*	file file_entropy block_count min_entropy max_entropy
/*
/*	Fields are tab separated.
/*
/* .IP \fIfile\fR
/*	The name of the file.
/* .IP \fIfile_entropy\fR
/*	The entropy computed over the entire file contents.
/* .IP \fIblock_count\fR
/*	The number of \fIblock_size\fR blocks used for the minimal and
/*	maximal per-block entropy computation.
/* .IP "\fImin_entropy, max_entropy\fR"
/*	The minimal and maximal per-block entropy for the named file.
/*	No per-block entropy is computed for partial blocks.
/* .PP
/*	 Options:
/* .IP "\fB-b \fIblock_size\fR (default: 8192)"
/*	The block size to be used for per-block entropies.
/* BUGS
/*	Entropy values can be misleading.
/* .IP \(bu
/*	The per-file entropy computed over the entire file is larger
/*	than any per-block entropy value for that same file.
/* .IP \(bu
/*	The entropy values as reported by this program are valid only
/*	for unstructured input. With real data such as text the result
/*	is an overestimate.
/* HISTORY
/* .fi
/* .ad
/*	This \fBentropy\fR command was written for the coroner's toolkit.
/* LICENSE
/*	This software is distributed under the IBM Public License.
/* SEE ALSO
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
#include <math.h>

/* SunOS4 needs this - I hope it does not break on some pedantic systems. */

extern char *optarg;
extern int optind;

/* Utility library. */

#include <error.h>

/* Application-specific. */

#define ALL_BYTES	256
#define DEF_BLOCK_SIZE	8192

double  log_2;

/* shannon - entropy for a sequence of bytes */

double  shannon(unsigned byte_frequency[ALL_BYTES], unsigned data_length)
{
    double  probability;
    double  sum;
    int     i;

    for (sum = 0, i = 0; i < ALL_BYTES; i++) {
	if (byte_frequency[i]) {
	    probability = (double) byte_frequency[i] / data_length;
	    sum -= probability * log(probability);
	}
    }
    return (sum / log_2);
}

/* process_file - process file one block at a time */

void    process_file(char *file, FILE *fp, int block_length)
{
    unsigned block_frequency[ALL_BYTES];
    unsigned file_frequency[ALL_BYTES];
    unsigned block_count;
    double  max_entropy = 0;
    double  min_entropy = 8;
    double  entropy;
    unsigned i;
    int     ch;

    /*
     * Initialize.
     */
    for (i = 0; i < ALL_BYTES; i++)
	file_frequency[i] = 0;

    /*
     * Process the file, one block at a time.
     */
    for (block_count = 0; /* void */ ; block_count++) {

	/*
	 * Initialize.
	 */
	for (i = 0; i < ALL_BYTES; i++)
	    block_frequency[i] = 0;

	/*
	 * Count character frequencies. Do not report incomplete blocks.
	 */
	for (i = 0; i < block_length; i++) {
	    if ((ch = getc(fp)) == EOF)
		goto done;
	    block_frequency[ch]++;
	    file_frequency[ch]++;
	}

	/*
	 * Compute block entropy.
	 */
	entropy = shannon(block_frequency, block_length);
	if (entropy > max_entropy)
	    max_entropy = entropy;
	if (entropy < min_entropy)
	    min_entropy = entropy;
    }
done:
    if (ferror(fp)) {
	if (errno == EISDIR) {			/* $%@! pedantic Linux */
	    remark("read %s: %m", file);
	    return;
	} else
	    error("read %s: %m", file);
    }

    /*
     * Compute file entropy and report results.
     */
    entropy = shannon(file_frequency, block_count * block_length + i);
    if (block_count == 0)
	min_entropy = max_entropy = 0;
    printf("%s\t%.2f\t%u\t%.2f\t%.2f\n", file, entropy,
	   block_count, min_entropy, max_entropy);
}

/* main - driver */

int     main(int argc, char **argv)
{
    FILE   *fp;
    int     block_length = DEF_BLOCK_SIZE;
    int     ch;
    int     i;

    progname = argv[0];

    /*
     * Parse JCL.
     */
    while ((ch = getopt(argc, argv, "b:")) > 0) {
	switch (ch) {
	case 'b':
	    if ((block_length = atoi(optarg)) <= 0)
		error("bad block size value: %s", optarg);
	    break;
	default:
	    error("usage: %s [-b block length] [file...]", progname);
	}
    }

    /*
     * Sanity checks.
     */

    /*
     * Process files.
     */
    log_2 = log(2.0);

    if (argc == optind) {
	process_file("(stdin)", stdin, block_length);
    } else {
	for (i = optind; i < argc; i++) {
	    if ((fp = fopen(argv[i], "r")) == 0) {
		remark("open %s: %m", argv[i]);
	    } else {
		process_file(argv[i], fp, block_length);
		(void) fclose(fp);
	    }
	}
    }
    exit(0);
}

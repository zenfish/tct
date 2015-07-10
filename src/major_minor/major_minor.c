/*++
/* NAME
/*	major_minor 1
/* SUMMARY
/*	PERL start engine
/* SYNOPSIS
/*	\fBeval\fR `\fBmajor_minor\fR`;
/*
/*	\fB$major_number = &dev_major($devnum);\fR
/*
/*	\fB$minor_number = &dev_minor($devnum);\fR
/* DESCRIPTION
/*	\fBmajor_minor\fR emits two PERL routines, \fBdev_major\fR()
/*	and \fBdev_minor\fR() that take a device number as returned
/*	by \fBstat\fR() and that break the number up into the device
/*	major and minor number, respectively.
/*
/*	For example, the FreeBSD 2.2 output is:
/*
/* .DS
/*	sub dev_major { local($dev) = @_; ($dev >> 8) & 0xff; };
/*
/*	sub dev_minor { local($dev) = @_; ($dev & 0xffff00ff); };
/*
/*	1;
/* .DE
/*
/*	In principle, this information could be obtained by parsing
/*	system include files, but a little C program is more robust.
/* HISTORY
/* .fi
/* .ad
/*	A similar trick was used first with the SATAN program.
/* LICENSE
/*	This software is distributed under the IBM Public License.
/* AUTHOR(S)
/*	Wietse Venema
/*	IBM T.J. Watson Research
/*	P.O. Box 704
/*	Yorktown Heights, NY 10598, USA
/*--*/

#include <sys/types.h>
#ifdef SUNOS5
#include <sys/mkdev.h>
#define TEMPLATE (NODEV>>1)
#endif
#ifdef LINUX2
#include <sys/sysmacros.h>
#endif
#ifndef TEMPLATE
#define TEMPLATE 0xffffffff
#endif

#include <stdio.h>

int     main(int argc, char **argv)
{
    unsigned foo = TEMPLATE;
    unsigned min = minor(foo);
    unsigned shift;
    unsigned mask;

    for (mask = ~min, shift = 0; (mask & 1) == 0; shift++, mask >>= 1)
	 /* void */ ;

    printf("sub dev_major { local($dev) = @_; ($dev >> %d) & 0x%x; };\n\n",
	   shift, major(foo));
    printf("sub dev_minor { local($dev) = @_; ($dev & 0x%x); };\n\n",
	   min);
    printf("1;\n");
    return (0);
}

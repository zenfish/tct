#
#  All code here was taken from File::PathConvert, at:
#
# http://www.oasis.leo.org/perl/exts/filehandling/File-PathConvert.dsc.html
#
#  I ripped out the realpath stuff, made it a normal subroutine instead
# of all that module crap, fixed some spelling, added a one-line cwd
# function (for older perls) and otherwise changed it very slightly.  Thanks
# to Shigio for the code!  Original copyright:
#
# 	Copyright (c) 1996 Shigio Yamaguchi. All rights reserved.
# 	This program is free software; you can redistribute it and/or
#	modify it under the same terms as Perl itself.
#
#				23-Oct-1996 Shigio Yamaguchi
#
#
#  (last mods by zen@fish.com, mar 28, 2000)
#

require "command.pl";

@ISA = qw(Exporter);
@EXPORT_OK = qw(realpath abs2rel rel2abs);

#
# instant configuration
#
$maxsymlinks = 32;		# allowed symlink number in a path
# $debug = 0;			# 1: verbose on, 0: verbose off
$SL = '/';			# separator

#
# realpath: returns the canonicalized absolute path name
#
# Interface:
#	i)	$path	path
#	r)		resolved name on success else undef
#	go)	$resolved
#			resolved name on success else the path name which
#			caused the problem.
	$resolved = '';
#
#	Note: this implementation is based 4.4BSD version realpath(3).
#
sub realpath {
    ($resolved) = @_;
    my($backdir) = &cwd();
    my($dirname, $basename, $links, $reg);

    regularize($resolved);
LOOP:
    {
	#
	# Find the dirname and basename.
	# Change directory to the dirname component.
	#
	if ($resolved =~ /$SL/) {
	    $reg = '^(.*)' . $SL . '([^' . $SL . ']*)$';
	    ($dirname, $basename) = $resolved =~ /$reg/;
	    $dirname = $SL if (!$dirname);
	    $resolved = $dirname;
	    unless (chdir($dirname)) {
		warn("realpath: chdir($dirname) failed.") if $debug;
		chdir($backdir);
		return undef;
	    }
	} else {
	    $dirname = '';
	    $basename = $resolved;
	}
	#
	# If it is a symlink, read in the value and loop.
	# If it is a directory, then change to that directory.
	#
	if ($basename) {
	    if (-l $basename) {
		unless ($resolved = readlink($basename)) {
		    warn("realpath: readlink($basename) failed.") if $debug;
		    chdir($backdir);
		    return undef;
		}
		$basename = '';
		if (++$links > $maxsymlinks) {
		    warn("realpath: too many symbolic links.") if $debug;
		    chdir($backdir);
		    return undef;
		}
		redo LOOP;
	    } elsif (-d _) {
		unless (chdir($basename)) {
		    warn("realpath: chdir($basename) failed.") if $debug;
		    chdir($backdir);
		    return undef;
		}
		$basename = '';
	    }
	}
    }
    #
    # Get the current directory name and append the basename.
    #
    $resolved = &cwd();
    if ($basename) {
	$resolved .= $SL if ($resolved ne $SL);
	$resolved .= $basename
    }
    chdir($backdir);
    return $resolved;
}

#
# regularize a path.
#
sub regularize {
    my($reg);

    $reg = '^' . $SL . '\.\.' . $SL;
    while ($_[0] =~ /$reg/) {           # ^/../ -> /
        $_[0] =~ s/$reg/$SL/;
    }
    $reg = $SL . '\.' . $SL;
    while ($_[0] =~ /$reg/) {
        $_[0] =~ s/$reg/$SL/;           # /./ -> /
    }
    $reg = $SL . '+';
    $_[0] =~ s/$reg/$SL/g;              # ///  -> /
    $reg = '(.+)' . $SL . '$';
    $_[0] =~ s/$reg/$1/;                # remove last /
    $reg = '(.+)' . $SL . '\.$';
    $_[0] =~ s/$reg/$1/g;               # remove last /.
    $_[0] = '/' if $_[0] eq '/.';
}

sub cwd {
$string = &command_to_string($PWD);
chop($string);
return($string);
}

1;

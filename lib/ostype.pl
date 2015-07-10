#!/bin/perl
#
#  Simple test that uses uname to determine what we're running on...
#

sub determine_os {

print "Determining OS (in determine_os())\n" if $verbose;

# Order does not matter, table is sorted.
%OS_INFO = (
    "FREEBSD2", "FreeBSD.2",
    "FREEBSD3", "FreeBSD.3",
    "FREEBSD4", "FreeBSD.4",
    "FREEBSD5", "FreeBSD.5",
    "FREEBSD6", "FreeBSD.6",
    "FREEBSD7", "FreeBSD.7",
    "OPENBSD2", "OpenBSD.2",
    "OPENBSD3", "OpenBSD.3",
    "OPENBSD4", "OpenBSD.4",
    "BSDI2", "BSD\/OS.2",
    "BSDI3", "BSD\/OS.3",
    "BSDI4", "BSD\/OS.4",
    "SUNOS4", "SunOS.4",
    "SUNOS5", "SunOS.5",
    "LINUX2", "Linux.2",
);

if (!$CORPSE) {
	chop($SYSTEM = &command_to_string($UNAME, "-s"));
	chop($RELEASE = &command_to_string($UNAME, "-r"));

	for (sort keys %OS_INFO) {
		if ("$SYSTEM.$RELEASE" =~ /$OS_INFO{$_}/) { $OS = "$_"; }
	}
	if (!$OS) { $OS = "unknown operating system"; }

	# return $OS;
	}

if (!$OS_INFO{$OS}) {
	print <<EOLIST;
Unknown operating system ($OS) - choose one from the following list:

EOLIST
	for (sort keys %OS_INFO) { print "\t$_\n"; }
	die;
	}

print "OS is: $OS\n" if $verbose;

}

# Scaffolding for stand-alone testing...

if ($running_under_grave_robber) {
	require "command.pl";
} else {
	$running_under_grave_robber = 1;
	require "../conf/coroner.cf";
	require "command.pl";
	&determine_os() . "\n";
	print $OS . "\n";
}

1;

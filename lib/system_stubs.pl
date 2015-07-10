#!/usr/local/bin/perl

#
# a simple set of stub routines to call the real functions that do
# host and network commands, based on OS type...
#

sub suck_netinfo {

print "going into suck_netinfo($OS)\n" if $verbose;

if ($OS =~ /BSD/) {
	require "bsd.pl";
	suck_netinfo_bsd();
	}
elsif ($OS eq "SUNOS4") {
	require "sunos.pl";
	suck_netinfo_sunos();
	}
elsif ($OS eq "SUNOS5") {
	require "solaris.pl";
	suck_netinfo_solaris();
	}
elsif ($OS =~ /LINUX/) {
	require "linux.pl";
	suck_netinfo_linux();
	}

else { die "Gack!  Unknown operating system ($OS)\n"; }

}

sub suck_hostinfo {

print "going into suck_hostinfo($OS)\n" if $verbose;

if ($OS =~ /BSD/)	{
	require "bsd.pl";
	suck_hostinfo_bsd();
	}
elsif ($OS eq "SUNOS4")	{
	require "sunos.pl";
	suck_hostinfo_sunos();
	}
elsif ($OS eq "SUNOS5")	{
	require "solaris.pl";
	suck_hostinfo_solaris();
	}
elsif ($OS =~ /LINUX/)	{
	require "linux.pl";
	suck_hostinfo_linux();
	}

else { die "Gack!  Unknown operating system ($OS)\n"; }

}

1;

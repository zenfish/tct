#
#  the lsof/icat approach won't work on linux (and maybe others)... plus
# they have the /proc file system, where we can simply copy stuff from.
#
# The OS's that use /proc and we can copy files from are:
#
#	SUNOS5		/proc/pid/object/a.out (SunOS 2.6 and later)
#	FREEBSD2	/proc/pid/file
#	FREEBSD3	/proc/pid/file
#	LINUX2		/proc/pid/exe
#
require "date.pl";

#
# copies a single file from /proc to the $PROC_FILES dir
#
sub cp_from_proc {
local($pid) = @_;

print "trying to copy $pid from /proc on a $OS system (in &cp_from_proc)\n" if $verbose;

if (!($OS  =~ /FREEBSD2/ || $OS =~ /FREEBSD3/ || $OS =~ /LINUX2/ ||
   ($OS =~ /SUNOS5/ && $RELEASE >= /5.6/))) {
	print "Don't know how to use /proc on $OS, falling back to using ICAT\n"
		if $verbose;
	return 0;
	}

mkdir("$PROC_FILES", 0700);

#
#	FREEBSD2	/proc/pid/file
#
if ($OS  =~ /FREEBSD2/ || $OS =~ /FREEBSD3/) {
	print "hmm, Freebsd, proc in /proc/pid/file => $PROC_FILES/$pid.out.\n" if $debug;
	&execute_command($CP, "/proc/$pid/file", "$PROC_FILES/$pid.out.\_$pretty_date");
	&sign_it("$PROC_FILES/$pid.out.\_$pretty_date");
	}

#
#	SUNOS5		/proc/pid/object/a.out (SunOS 2.6 and later)
#
elsif ($OS =~ /SUNOS5/) {
	print "hmm, solaris >= 2.6, proc in /proc/pid/object/a.out => $PROC_FILES/$pid.out.\n" if $debug;
	&execute_command($CP, "/proc/$pid/object/a.out","$PROC_FILES/$pid.out.\_$pretty_date");
	&sign_it("$PROC_FILES/$pid.out.\_$pretty_date");
	}

#
#	LINUX2		/proc/pid/exe
#
elsif ($OS =~ /LINUX2/) {
	print "hmm, linux, proc in /proc/pid/exe => $PROC_FILES/$pid.out.\n" if $debug;
	&execute_command($CP, "/proc/$pid/exe","$PROC_FILES/$pid.out.\_$pretty_date");
	&sign_it("$PROC_FILES/$pid.out.\_$pretty_date");
	}
else {
	warn "this shouldn't happen... bleah!  (&cp_from_proc, $OS)\n";
	}

return 1;
}

#
#  Calls cp_from_proc() for each pid...
#
sub cp_all_from_proc {

# might need this for output file, should already be calculated
&get_date();

if (defined(%lsof_pids)) {
	print "USING LSOF!\n" if $debug;
	for (keys %lsof_pids) {
		print "pid: $_\n" if $debug;
		print "saving process $_ from /proc...\n" if $ debug;
		return 0 unless cp_from_proc($_);
		}
	}
else {
	print "USING LSOF!\n" if $debug;
	for (keys %ps_pids) {
		print "pid: $_\n" if $debug;
		print "saving process $_ from /proc...\n" if $ debug;
		return 0 unless cp_from_proc($_);
		}
	}
return 1;
}

if (!$running_under_grave_robber) {
	require "lib/ostype.pl";
	require "lib/command.pl";
	require "lib/dig-sig.pl";
	require "lib/ps_spy.pl";

	$running_under_grave_robber = 1;

	&determine_os();
	&suck_ps();

	$verbose = $debug = 1;

	&cp_all_from_proc();
	}

1;

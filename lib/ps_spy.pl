# 
#   ps_spy.pl - process spy library.  Part of the coroner's toolkit.
#

# print "DEBUG  - $debug\n";

#  So far:
#
# suck_lsof() 	Slices 'n' dices lsof output into:
#
#			%pid{$pid, $comm} = $name;
#			%full_path{$pid}  = $name;
#			%lsof_pids{$pid}  = $pid;
#
#			(name is full pathname of comm component)
#
# suck_ps() 	Slices 'n' dices ps output into:
#
#			%ps_pids{$pid} = $pid;
#
# save_open_files() 	Uses pcat to save all files that are open &
#			running but have their executable deleted.
#
# suck_proc_pcat() 	Uses pcat to suck in *all* running processes; saves
#			them in $DATA/pcat/$pid.out
#
# suck_proc_icat() 	Uses icat to suck in all running processes; saves
#			them in $DATA/icat/$pid.out
#
# suck_inetd()	Sucks up /etc/inetd.conf or /etc/services into:
#
#			%services{$service} = $service;
#
# check_ttys()	Flags any programs in the @no_ttys variable (in 
#		coroner.cf) and that have a PID < 100 that have a tty
#		(/dev/ttyp*, /dev/tty*) connected with them
#
# check_nit()	Prints out any process that has opened a file in the @NIT var
#		(in coroner.cf); currently this is simply /dev/nit, but 
#		ideally any network interface.
# 

#
#  Other notes, thoughts:
#
# Try to examine all those processes running around in process-land.
# Uses lsof, ps, and/or anything else I can think of.
#
# ... try to focus on
# finding something that shouldn't be there.  Odd for some reason;
# was reading the network interface, waiting for network traffic on a
# non-standard port, having a control terminal/tty when there shouldn't
# be one or vice-versa, etc.
#  
# ... list of "expected" ports in use and have an lsof|perl
# combination that reports on anything outside that list.  It might be possible
# to generate part of the expected port list from /etc/inetd.conf...?
#  
# Lsof can usually tell you what shared libraries the process is using.
#
#
#

require "tm_misc.pl";
require "date.pl";
require "rawdev.pl";

sub suck_lsof {

print "running the lsof tool... this might take a bit of time (in suck_lsof())\n" if $verbose;

#
#  Use the -F flag, 'cause lsof gives interesting output; the 0 means
# separate fields by nulls.
#

return unless -x $LSOF;

&pipe_command(LSOF, $LSOF, "-F0", "-|");

&date_stamp("$COMM_OUT/lsof0");
die "Can't write $DATA/$LSOF (in suck_lsof())\n" unless open(LSOF_OUT, ">>$COMM_OUT/lsof0");

#
#  First, suck in the info.  Even on my sparc2 this is > 700 lines of output...
#
while (<LSOF>) {
	print LSOF_OUT $_;
	chop;
	@fields = split(/\0/, $_);

	if (/^p/) {

# if ($lson) { print "FOUND $comm!\n";}
if (!$lson && $comm) { print "Didn't find $comm!\n" if $debug;}
$lson = 0;
		print "PPPPPPPPPPPPPPProcess group...\n" if $debug; 

		#
		#  It can be either p, c, u, L, g, or R
		#
		for $field (@fields) {
			if    ($field =~ /^p/) {
				($pid)   = ($field =~ /^p(\S*)/);
				print "Setting the PID to $pid\n" if $debug;
				print "\t$_\n" if $debug;
				if ($pid) {
					$all_pids{$pid} = $pid;
					$lsof_pids{$pid}= $pid;
					print "PID: $pid " if $debug;
					}
				}
			elsif ($field =~ /^c/) {
				($comm)  = ($field =~ /^c(\S*)/);
				print "COMM: $comm " if $debug && $comm;

				# when we find the inode for this, check
				# it out...
				$check_command = 1;
				}
			elsif ($field =~ /^u/) {
				($uid)   = ($field =~ /^u(\S*)/);
				print "UID: $uid " if $debug && $uid;
				}
			elsif ($field =~ /^L/) {
				($login) = ($field =~ /^L(\S*)/);
				print "Login: $login " if $debug && $login;
				}
			elsif ($field =~ /^g/) {
				($pgrp)  = ($field =~ /^c(\S*)/);
				print "PGRP: $pgrp " if $debug && $pgrp;
				}
			elsif ($field =~ /^R/) {
				($ppid)  = ($field =~ /^c(\S*)/);
				print "PPID: $ppid " if $debug && $ppid;
				}
			else	{ print "ERROR in PROCESS GROUP: ... $field ...  "
				  if $debug; }
			}
		}

	elsif (/^f/) {
		$state = "";
		# print "\tFile group...\n"; 
		for $field (@fields) {
			print "\nKKK: $field\n" if $debug;
			if    ($field =~ /^n/) {
				($name) = ($field =~ /^n(\S*)/);
				print "\tNAME: $name " if $debug && $name;

				print "\tNAME: $name#$comm\n" if $debug;

				$pids{"$pid,$name"} = $comm;
				}
			elsif ($field =~ /^a/) {
				($access) = ($field =~ /^a(\S*)/);
				print "\tAccess: $access " if ($debug && $access);
				}
			elsif ($field =~ /^l/) {
				($lock)   = ($field =~ /^l(\S*)/);
				print "\tLock: $lock " if $debug && $lock;
				}
			elsif   ($field =~ /^d/) {
				($devch)  = ($field =~ /^d(\S*)/);
				print "\tDevCh: $devch " if $debug && $devch;
				}
			elsif   ($field =~ /^D/) {
				($devnum) = ($field =~ /^D(\S*)/);
				print "\tDevNum: $devnum FD($fd) " if $debug && $devnum;

				#
				# might need to check out the txt segment for
				# the icat sucker.  This appears to
				# come after fd in lsof output...
				#
				if ($fd eq "txt" && !defined($ls_of_txt{$pid})){
					$devnum = sprintf("%d", hex($devnum));
					$maj = &dev_major($devnum);
					$min = &dev_minor($devnum);

					$ls_of_txt{$pid} = "$maj,$min";
					printf("DEVNUM: %d ($maj/$min)\n",  hex($devnum))
						if $debug;
					}
				}
			elsif   ($field =~ /^f/) {
				($fd)  = ($field =~ /^f(\S*)/);
				print "\tFD: $fd " if $debug && $fd;

				}
			elsif   ($field =~ /^i/) {
				($inode)  = ($field =~ /^i(\S*)/);
				print "\tInode: $inode " if $debug && $inode;
# print "NAME: $inode $name\n" if ($name !~ m@^/.*@);
print "XXXX: $comm - $inode - $name\n" if $debug;
				if ($fd eq "txt" && defined($ls_of_txt{$pid})
					&& !defined($ls_of_i{$pid})) {
					$ls_of_i{$pid} = $inode;
					}

				if ($check_command) {
					# &match_md5($comm);
					$check_command = 0;
					}
				}
			elsif   ($field =~ /^s/) {
				($size)   = ($field =~ /^s(\S*)/);
				print "\tSize: $size " if $debug && $size;
				}
			elsif   ($field =~ /^o/) {
				($off)    = ($field =~ /^o(\S*)/);
				print "\tOffset: $off " if $debug && $off;
				}
			elsif   ($field =~ /^P/) {
				($proto)  = ($field =~ /^P(\S*)/);
				print "\tProto: $proto " if $debug && $proto;
				}
			elsif   ($field =~ /^S/) {
				($stream) = ($field =~ /^S(\S*)/);
				print "\tStream: $stream " if $debug && $stream;
				}
			elsif   ($field =~ /^t/) {
				($type)   = ($field =~ /^t(\S*)/);
				print "\tType: $type " if $debug && $type;
				}
			elsif   ($field =~ /^T/) {
				($s)      = ($field =~ /^T(\S*)/);
				if (!$state) {
					$state = "(" . $s;
					}
				else {
					$state .= " " . $s;
					}
				}
			else	{ print "ERROR in FILE GROUP: ... $field ...  "
				  if $debug; }
			}
		}

	$full_path{$pid} = $name;

	print "State: $state) " if $debug && $state;
	print "\n" if $debug;
	}

close(LSOF);
close(LSOF_OUT);
&sign_it("$COMM_OUT/lsof0");

#
#   Save it a second time, this time in text format, not with the -F0 field...
# easier to read.
#
&date_stamp("$COMM_OUT/lsof");
&redirect_command($LSOF, ">>$COMM_OUT/lsof");
&sign_it("$COMM_OUT/lsof");

}

#
#  Not sure what to do with this yet ;-)
#
sub suck_inetd {
local($service);

print "sucking in the inetd file (in suck_inetd())\n" if $verbose;

# If sysV based
if (-f "/etc/servers") {
	die "Can't open /etc/servers (in suck_inetd())\n" unless open(SERVERS, "/etc/servers");
	while (<SERVERS>) {
		next if (/^\s*#/ || /^\s*$/);
		($a,$b,$service,$c) = split();
		$services{$service} = $service;
		print "SERVERS: $service ($_)\n" if $debug;
		}
	}
 
# else BSD (e.g. the right way :-))
elsif (-f "/etc/inetd.conf") {
	die "Can't open /etc/inetd.conf (in suck_inetd())\n" unless 
		open(INETD, "/etc/inetd.conf");
	while (<INETD>) {
		next if (/^\s*#/ || /^\s*$/);
		($a,$b,$c,$d,$e,$service,$f) = split();
		$services{$service} = $service;
		print "INETD: $service ($_)\n" if $debug;
		}
	}

}

#
#  The network interface shouldn't be opened for reading, writing,
# or 'rithmitic.
#
sub check_nit {

for $nit (@NIT) {
	if (-e $nit) {
		# print "NO! -> $nit\n";
		$host_nits .= " $nit";
		}
	}

# $result = `$LSOF $host_nits 2> /dev/null`;
$result = &command_to_string($LSOF, $host_nits);

print "The following network interface devices are open!\n\n$result\n";

}

#
# Look at all PID's - if < $low_pid or the current prog is =~ /dev/ttyp*
# (calling subs above) then scream...
#
sub check_ttys {

local($p,$c,$program);

#
#  Now go over all the lsof pids
#
for $program (keys %pids) {
	($p, $c) = split(/,/, $program);
	print "Process: $program @ $p @ $c @ $pids{$program}\n" if $debug;

	if ($c =~ m@/dev/ttyp@ || $c =~ m@/dev/tty[A-z]@) {
		if ($p < $low_pid) {
			print "Process $pids{$program} (PID $p) has a tty ($c) associated with it!\n";
			}
		elsif ($no_ttys{$pids{$program}}) {
			print "SERVICE $pids{$program} has a tty ($c) associated with it!\n";
			}
		}
	$all_pids2{$p} = $p;

	if (!defined($ps_pids{$p})) {
		print "Hey!  PID: $p is in $LSOF but not in $PS Output!\n" if $debug;
		print "\t$pids{$program} @ PID $p @ $c\n";
		print "\n";
		}
	else {
		# print "PIDINFO: $ps_pids{$p}\n";
		}
	}
}

#
# Slices 'n' dices ps output, puts the environ of process in %ps_pids
# if available, else just the PID.
#
# Since bsd/sysV ps's take different args, we need to determine what
# is the type of ps we're using.  I check out the first line of a normal 
# (no args) "ps" call; sysV appears to have something like:
#
#	   PID TT       S  TIME COMMAND
#
# SunOS/BSD(?) has something like:
#
#	  PID TT STAT  TIME COMMAND
#
# I simply search for "STAT" - if not there, assume sysV-like ps.
#
#		%ps_pids{$pid} = $pid;
#
# If BSD, I use "-auxwwge", which also dumps out environ info.  Empirically
# there appears to be 12 or more fields in the output (after the header).
# There may be 11 (or I could be full of shit.)  I shall test more.
#
# If SysV, I simply use "-A" (all processes report)
#
sub suck_ps {
local($first_line, $args, $pid);

print "running ps... (in suck_ps())\n" if $verbose;

&pipe_command(PS, $PS, "-|");

$first_line = <PS>;
close(PS);

if ($first_line =~ /STAT/) {
	print "BSD Universe\n" if $debug;
	$bsd  = 1;
	$sysv = 0;
	$args  = "auxwwge";

	if ($CORPSE && $OS eq "SUNOS4") { $args = "auxwwgek"; }

	if ($OS eq "LINUX2") { $args  = "auxwwe"; }
	}
else {
	print "SysV Universe\n" if $debug;
	$bsd  = 0;
	$sysv = 1;
	$args  = "A";
	}
#
# Now do the real ps, with full-blown args
#
if (!$CORPSE) {
	&pipe_command(PS, $PS, "-$args", "-|");
	}
elsif ($CORPSE && $OS eq "SUNOS4") {
	&pipe_command(PS, $PS, "-$args", "-|");
	}
else {
	return;
	}

&date_stamp("$COMM_OUT/ps");
die "Can't write $DATA/$PS (in suck_ps())\n" unless open(PS_OUT, ">>$COMM_OUT/ps");

#
# Throw away the first line, just headers
#
$first_line = <PS>;

while (<PS>) {
	print PS_OUT $_;
	print "PS type $sysv/$bsd, $_\n" if $debug;
	chop;
	if ($sysv) {
		print "PS line = $_ =\n" if $debug;

		($pid = $_) =~ s/^\s*([0-9]+)\s.*$/$1/;

		$ps_pids{$pid} = $pid;
		print "PS <== $_ ==> \n### $pid ###\n" if $debug;
		}
	elsif ($bsd) {
		$pid_num = 0;
		@pid = ("");
		$environ = $pid = "";

		@pid = split();
		# print "Process environ:\n";
		for (@pid) {
			# get the PID first
			print "next... $_ $environ ___ \n" if $debug;
			if ($pid_num == 1) {
				$pid = $_; 
				print "next pid... $pid\n" if $debug;
				$pid_num++;
				next; 
				}

			#
			# Count off the magic num of fields
			#
			next if ($pid_num++ < ($magic_ps_fields - 1));

			# what's left?
			print "$_ " if $debug;
			$environ .= "$_ ";
			}
		print "\n" if $debug;
		print "Hey!  NOTHING in environ var in BSD Universe ($pid_num):\n\t$_\n"
			if (!$environ && $debug);
		$ps_pids{$pid} = $environ;
		}
	else { die "Hey!  BSD/SysV universe not set!\n"; }
	}

close(PS);
close(PS_OUT);
&sign_it("$COMM_OUT/ps");

}

#
# This looks at the command in question and tries to find its location
# on the system by looking at standard system dirs; /bin, /usr/bin, etc.
# (This is maintained in coroner.cf), then does an md5 on it.  It then 
# attempts to look at the inode of the command in memory & see if it 
# matches the one found.
#
# Gripes if there are > 1 executables with the same name in the paths, 
# & further gripes if they aren't the same executable (compare using md5).
#
sub match_md5 {
local($exe) = @_;
local($path, $md5, $p1, $p2);

return if (!$exe);

for $path (@system_paths) {
	if (-x "$path/$exe" && !-l "$path/$exe") {
		if (!-r "$path/$exe") {
			print "Can't get MD5 for $path/$exe - is unreadable!\n";
			return "unreadable";
			}
		$md5 = &md5("$path/$exe");

		if (defined($exes{$exe}) && ($paths{$exe} ne $path)) {
			($p1, $p2) = split(/\s/, $md5);
			print "Hey!  $exe is located in $paths{$exe} AND $path!\n" if $debug;
			if ($p1 ne $md5s{$exe}) {
				print "\tThe two executables ($exe) are different!\n\t\t$p1\n\t\t$md5s{$exe}\n";
				return "different";
				}
			}
		$found = 1;
		($p1, $p2) = split(/\s/, $md5);
		$exes{$exe}  = $exe;
		$paths{$exe} = $p2;
		$md5s{$exe}  = $p1;
		print "Found $exe - $md5\n" if $debug;
		print "\n\t\tFound $exe - $md5 - $inode\n" if $debug;
		$lson = 1;
		last;
		}
	}

if (!$found) {
	print "Could not find binary for $exe\n" if $debug;
	return "";
	}

return $md5s{$exe};

}

#
# save_open_files() 	Uses pcat to save all files that are open &
#			have been deleted.
#
sub save_open_files {

# might need this for output file, should already be calculated
&get_date();

#
# Need ils and icat
#
if (-x $ILS && -x $ICAT && -x $DF) {
	my(@df) = "df";

	if ($OS eq "SUNOS5") { push(@df, "-k"); }

	&pipe_command(DF, @df, "-|");

	while (<DF>) {
		next if (/Filesystem/);
		$_ .= <DF> unless (/ /);
		print $_ if $debug;
		chop;
		($dev,$x) = split(/\s/, $_);
		$dev = &rawdev($dev);
		push(@devs, $dev);
		}
	close(DF);

	mkdir($DELETED_FILES, 0700);
	command_to_string("$SYNC");

	for $dev (@devs) {
		&pipe_command(ILS, $ILS, "-o", "$dev", "-|");

		#
		# used below to save to a safe name - can't save stuff
		# with '/'s in the filename!
		#
		($safe_dev = $dev) =~ s@/@_@g;

		# get rid of TM headers
		<ILS>;
		<ILS>;
		<ILS>;
		while (<ILS>) {
			($inode, @x) = tm_split($_);
			print "ILS: $inode => $_\n" if $debug;

			print "saving inode $inode to $DELETED_FILES/$safe_dev\_$inode.out\_$pretty_date...\n" if $debug;

			&redirect_command($ICAT, $dev, $inode, ">$DELETED_FILES/$safe_dev\_$inode.out\_$pretty_date");
			# if an error, get rid of the stupid file
			unlink("$DELETED_FILES/$safe_dev\_$inode.out\_$pretty_date") if ($?);
			command_to_string("$SYNC");
			}
		close(ILS);
		}
	}

}


#
# suck_proc_pcat() 	Uses pcat to suck in *all* running processes; saves
#			them in $DATA/pcat/$pid.out
#
#
sub suck_proc_pcat {

# might need this for output file, should already be calculated
&get_date();

print "grabbing all processes found using pcat... (in suck_proc_pcat())\n" if $verbose;

#
# If we have pcat, use that...
#
if (-x $PCAT) {
	mkdir("$DATA/pcat", 0700);
	#
	# use lsof data if possible, else ps
	#
	if (defined(%lsof_pids)) {
		print "USING LSOF!\n" if $debug;
		for (keys %lsof_pids) {
			print "LSOF: $_\n" if $debug;
			print "saving process $_ to $DATA/pcat/$_.out...\n" if $debug;
			print "saving mapname $_ to $DATA/pcat/$_.map...\n" if $debug;
			&redirect_command($PCAT, "-m", "$DATA/pcat/$_.map.$pretty_date", $_, ">$DATA/pcat/$_.out.$pretty_date");
			&sign_it("$DATA/pcat/$_.out.$pretty_date");
			&sign_it("$DATA/pcat/$_.map.$pretty_date");
			}
		}
	else {
		for (keys %ps_pids) {
			print "PS: $_\n" if $debug;
			print "saving process $_ to $DATA/pcat/$_.out...\n" if $debug;
			print "saving mapname $_ to $DATA/pcat/$_.map...\n" if $debug;
			&redirect_command($PCAT, "-m", "$DATA/pcat/$_.map.$pretty_date", $_, ">$DATA/pcat/$_.out.$pretty_date");
			&sign_it("$DATA/pcat/$_.out.$pretty_date");
			&sign_it("$DATA/pcat/$_.map.$pretty_date");
			}
		}
	}
else {
	print "can't do pcat - no executable... (in suck_proc_pcat())\n"
		if $verbose;
	}
}

#
# suck_proc_icat() 	Uses icat to suck in all running processes; saves
#			them in $DATA/icat/$pid.out
#
# needs:
#	o	icat
#	o	%all_major_minor_[bc] (holds the maj/min #'s)
#	o	%ls_of_txt (holds the device the first txt is on
#
sub suck_proc_icat {
local($device);

# might need this for output file, should already be calculated
&get_date();

print "grabbing all processes found using icat... (in suck_proc_icat())\n" if $verbose;

#
# need:
#	o	icat
#	o	%all_major_minor_[bc] (holds the maj/min #'s)
#	o	%ls_of_txt (holds the device the first txt is on
#
if (-x $ICAT && defined(%all_major_minor_c) && defined(%ls_of_txt)) {
	mkdir("$ICAT_OUT", 0700);
	#
	# use lsof data if possible, else ps
	#
	print "USING LSOF!\n" if $debug;
	for $pid (keys %lsof_pids) {
		print "LSOF: $pid\n" if $debug;
		print "saving process $pid to $DATA/icat/$pid.out...\n" if $debug;
		$device = (defined($all_major_minor_b{$ls_of_txt{$pid}}) ?
			$all_major_minor_b{$ls_of_txt{$pid}} :
			$all_major_minor_c{$ls_of_txt{$pid}});
		print "$ICAT, -> $ls_of_txt{$pid} <-, $device, $ls_of_i{$pid},>$DATA/icat/$pid.out\n" if $debug;


		# &date_stamp("$DATA/icat/$pid.out");
		&redirect_command($ICAT,
			&rawdev($device),
			$ls_of_i{$pid},
			">$DATA/icat/$pid.out\_$pretty_date");
		&sign_it("$DATA/icat/$pid.out\_$pretty_date");
		}
	}
else {
	print "can't use icat... either no $ICAT or can't find maj/min stuff (in suck_proc_icat())\n" if $verbose;
	if ($debug) {
		print "can't find icat\n" if (!-x $ICAT);
		print "all_major_minor undef'd\n" if (!defined(%all_major_minor_c));
		print "ls_of_txt undef'd\n" if (!defined(%ls_of_txt));
		}
	}
}

if (!$running_under_grave_robber) {
	require "lib/ostype.pl";
	require "$LIB/dig-sig.pl";
	require "$LIB/maj_min_walk.pl";

	# $verbose = $debug = 1;
	# &suck_lsof();

	# need this for icat
	# &process_dev_dir($DEVICE_DIR);
	# &suck_proc_icat();

	# &save_open_files();
	# &suck_proc_pcat();
	}

1;

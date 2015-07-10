#
#  A quick hack to suck up all the trust on a system
#
#  Currently only grabs .rhosts, & .forward files, dumps crontab and 
# at output, and xhost/xauth results... should get pipes in alias files as 
# well, other stuff.  More windowing stuff should be added.
#

require "pass.cache.pl";
require "ostype.pl";

#
# cycle through the users, grab all rhosts files
#
sub grab_user_trust_files {

print "Grabbing all trust-related files (in grab_user_trust_files())\n" if $verbose;

for $user (keys %uname2dir) {

	print "\nChecking user $user..." if $debug;

	# lots of system stuff uses same dir, only need it once...?
	# next if (defined($all_rhosts_dirs{$uname2dir{$user}}));

	for $trust_files (@user_trust_files) {

		print "looking at: $uname2dir{$user}/$trust_files\n" if $debug;
		# wildcard expansion...
		for $trust_file (<$uname2dir{$user}/$trust_files>) {

			# we want to know even about empty ones...
			print "\t($trust_file)\n" if $debug;

			next unless -e $trust_file;

			print "TF: $trust_file\n" if $debug;

			#
			# ... is a real file, not a symlink
			#
			next if -l $trust_file && !$follow_file_sym_links;

			print "something in there...\n" if $debug;

			($faux_file = $trust_file) =~ s@/@_@g;

			$resting_place="$DATA/$user_vault/$user\_$faux_file";

			#
			# can't happen, but...
			#
			if ($trust_file eq $resting_place) {
				warn "Saving $trust_file would clobber it\n";
				next;
			}
			&date_stamp($resting_place);
			# use cat instead of cp because of date stamp
			&redirect_command($CAT, $trust_file, ">>$resting_place");
			&sign_it("$resting_place");
			}
		}
	}
}

#
#  Snarf the crontab and at commands for any users that use them...
#
sub grab_user_time_trust {

print "Grabbing all time stuff (at, cron, etc) (in grab_user_time_trust())\n" if $verbose;

&date_stamp("$TRUST/time");
die "Can't open $TRUST/time (in grab_user_time_trust())\n" unless open(TT, ">>$TRUST/trust.time");

#
# if can use the by-user crontab -l command...
#
if (-x $CRONTAB) {
	my(@crontab) = ("$CRONTAB", "-l");

	# sun doesn't use the -u option, others seem to...
	if (($OS ne "SUNOS4") && ($OS ne "SUNOS5")) { push(@crontab, "-u"); }

	for $user (keys %uname2dir) {
		print "\nChecking user $user\'s $CRONTAB & $AT..." if $debug;

		&pipe_command(CRONTAB, @crontab, "$user", "-|");

		if (!$?) {
			while (<CRONTAB>) {
				print "\t$user (cron): $_" if $debug;
				print TT "$user (cron): $_";
				}
			}
		close(CRONTAB);

		#
		# &pipe_command(AT, $SU, "$user", "-c", "$AT -l", "-|");
		if (!$?) {
			while (<AT>) {
				print "\t$user (at): $_" if $debug;
				print TT "$user (at): $_";
				}
			}
		close(AT);
		}
	}
#
# for those (4.4, etc?)
#

# no error checking; if doesn't exist/can't open, don't worry about it..
open(CRONTAB, $etc_crontab);
while (<CRONTAB>) {
	print 
	print "($etc_crontab): $_" if $debug;
	print TT "($etc_crontab): $_";
	}

close(TT);

&sign_it("$TRUST/time");

}

#
# just a simple-minded couple of things
#
sub grab_window_trust {

print "Grabbing some window stuff (in grab_window_trust())\n" if $verbose;

&date_stamp("$TRUST/window_systems");
die "Can't open $TRUST/window_systems (in grab_window_trust())\n"
	unless open(WINDOWS, ">>$TRUST/window_systems");

if (-x $XHOST) {
	&pipe_command(XHOST, $XHOST, "-|");
	while (<XHOST>) {
		print "(xhost): $_" if $debug;
		print WINDOWS "(xhost): $_";
		}
	}
close(XHOST);

if (-x $XAUTH) {
	&pipe_command(XAUTH, $XAUTH, "list", "-|");
	while (<XAUTH>) {
		print "(xauth): $_" if $debug;
		print WINDOWS "(xauth): $_";
		}
	}
close(XAUTH);
close(WINDOWS);

&sign_it("$TRUST/window_systems");

}

if (!$running_under_grave_robber) {

	print "testing out all the trust functions...\n";

	require "../conf/coroner.cf";

	&determine_os();
	require "lib/dig-sig.pl";

	# Load the password stuff
	&'load_passwd_info(0,$PASSWD);
	&'load_group_info(0,$GROUP);
	$verbose = $debug = 1;

	&grab_user_trust_files();
	# &grab_user_time_trust();
	# &grab_window_trust();
	}

1;

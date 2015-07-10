#
#  use ils to suck up all the free inode information
#

require "tm_misc.pl";
require "rawdev.pl";
require "crunch.pl";
require "suck_table.pl";

sub suck_free_inodes {

print "Grabbing all free inode info (in &suck_free_inodes())\n" if $verbose;

#
# hard part - find what's out there... need to consider $CORPSE, etc...
#
@all_disks = &find_disks() if (!defined(@all_disks));

#
# easy part, just cycle over disks, save info
#

for $disk (@all_disks) {
	&grab_inode_info($disk);
	}

}

#
# Currently just use DF, this has to change...
#

sub find_disks {

print "Trying to find all disks (in &find_disks())\n" if $verbose;

@devs = "";

if (-x $DF) {
	my(@df) = "df";

	if ($OS eq "SUNOS5") { push(@df, "-k"); }

	#
	# two trains of thought here.  If it's a corpse we're looking at,
	# then try to find the /dev that corresponds to that.  Else just
	# parse out things that look like things we can mess with
	#
	if ($CORPSE) {
		# &pipe_command(DF, @df, $CORPSE, "-|");
		&pipe_command(DF, @df, "-|");
		my($fixed_corpse) = $CORPSE;
		$fixed_corpse =~ s@/+$@@;
		$fixed_corpse .= "/";
		while (<DF>) {
			next if (/Filesystem/);
			$_ .= <DF> unless (/ /);
			chop;

			($dev,$a,$b,$c,$d,$mount_point) = split(/\s+/, $_);
			print "DF: $dev,$a,$b,$c,$d,$mount_point\n" if $debug;

			$mount_point =~ s@/+$@@;
			$mount_point .= "/";
			print "examining $mount_point vs $fixed_corpse\n" if $debug;

			#
			#   try to get all the dirs including & below the
			#  $CORPSE var... this gets, say, /foo, /foo/bar,
			# & /foo/foo, if $CORPSE eq /foo.
			#  
			if ("$mount_point" =~ /^$fixed_corpse/) {
				print "Corpse mount - $mount_point\n" if $debug;
				$dev = &rawdev($dev);
				push(@devs, $dev);
				}
			}
		}
	else {
		&pipe_command(DF, @df, "-|");
		while (<DF>) {
			next if (/Filesystem/);
			$_ .= <DF> unless (/ /);

			chop;

			($dev,$a,$b,$c,$d,$mount_point) = split(/\s+/, $_);
			print "DF: $dev,$a,$b,$c,$d,$mount_point\n" if $debug;

			$dev = &rawdev($dev);

			push(@devs, $dev);
			}
		}
	close(DF);

	return(@devs);
	}

}


#
# Use ils to save all inode info of unused inodes
#

sub grab_inode_info {
local($disk) = @_;

print "grabbing all unallocated inode info in $disk (in &grab_inode_info())\n" if $verbose;

return unless -e $disk;

($d = $disk) =~ s@/@_@g;

if (-x $ILS) {
	&date_stamp("$COMM_OUT/free_inode_info.$d");
	&redirect_command($ILS, $disk, ">>$COMM_OUT/free_inode_info.$d");
	&sign_it("$COMM_OUT/free_inode_info.$d");
	}

}

if (!$running_under_grave_robber) {
	# $debug = $verbose = 1;
	require "lib/command.pl";
	require "lib/dig-sig.pl";

	&suck_free_inodes();
	}

1;


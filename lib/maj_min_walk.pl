#
#  Simply prints out the major and minor device numbers to everything in
# a directory tree; typically called on /dev.
#
#  NOTE - uses stat, not lstat, on the files.
#

require "command.pl";
require "major_minor.pl";

#
#
#  This doesn't get called anymore... depricated.
#
# runs the major_minor C program to create the perl subroutines,
# also a sanity check... if it doesn't exist, squawk.
#
sub create_maj_min_stubs {

if (!-x $MAJ_MIN) {
	die "You must type \"reconfig\" and \"make\" before running this program!\n"
	}

# print "creating the major/minor stub functions (in create_maj_min_stubs())\n"
# 	if $verbose;

# print "COMM: " . &command_to_string($MAJ_MIN);
# eval &command_to_string($MAJ_MIN);
# die "$MAJ_MIN: $@.\n" if $@;

}

#
# climb the dev tree, make the maj/min #s...
#
sub process_dev_dir {
local($dir) = @_;
local(@dirs);

print "going into dev dir with \"$dir\"...\n" if $debug;
print "getting the major/minor numbers of $dir (in process_dev_dir())\n" if $debug;

if (defined($already_seen_dev_dir{$dir})) {
	print "We've already processed this directory ($dir), skipping...\n"
		if $debug;
	return;
	}

$already_seen_dev_dir{$dir} = 1;

if (!opendir(DIR, $dir)) {
	warn "Can't open $dir via opendir (in process_dev_dir())\n";
	return;
	}

#
# Suck in all the dir entries
@next = readdir(DIR);

#
#  If we're stupid enough to call this with a trailing slash in the
# filename, or if we call it with just "/", strip it off.
#
$dir =~ s@/$@@;

die "Can't open $maj_min_db (in process_dev_dir())\n" unless open(MAJ_MIN,">>$maj_min_db");

# 
# Go over each of the dir entries
#
while (($_ = pop(@next))) {
	print "next dir entry: $_\n" if $debug;

	# skip ".", "..", and null dir entries
	next if ($_ eq "\." || $_ eq "\.\." || ! $_);

	print "DIR: $dir\n" if $debug;

	#
	# Add the path to the filename
	$file = $_;	# (we'll need this in a bit)
	$_ = "$dir/$_";

	if (-d) { print "$_ Is -d\n" if $debug; push(@dirs, $_); }
	else  {
		#
		# in this code we always follow syms... might have to
		# get smarter later on, but for now...
		#
		#  next if (!$follow_file_sym_links && -l $_);

		next if (-f $_);	# skip MAKEDEV stuff...

		($st_dev,$st_ino,$st_mode,$st_nlink,$st_uid,$st_gid,
		 $st_rdev,$st_size,$st_atime,$st_mtime,$st_ctime,
		 $st_blksize,$st_blocks) = stat($_);

		#
		# Linux... *sigh*...
		#
		$st_blksize = 0 unless $st_blksize;
		$st_blocks = 0 unless $st_blocks;

		$dmaj = &dev_major($st_rdev);
		$dmin = &dev_minor($st_rdev);

		# what sort of device is it?
		$type = "c" if -c $_;
		$type = "b" if -b $_;
		$type = "!" if (!-c $_ && !-b $_);

		#
		# keep track of block/char devices so we can icat them later
		#
		$all_major_minor_b{"$dmaj,$dmin"} = $_ if $type eq "b";
		$all_major_minor_c{"$dmaj,$dmin"} = $_ if $type eq "c";

		print "$_ $type $dmaj $dmin\n" if $debug;

		print MAJ_MIN "$_ $type $dmaj $dmin\n";

		}
	}

closedir(DIR);
close(MAJ_MIN);

#
#  Process subdirs in the dir; strings, crunch, and search it for more dirs...
#
for (@dirs) {
	#
	# in this code we always follow syms... might have to
	# get smarter later on, but for now...
	#
	# next if (!$follow_dir_sym_links && -l $_);

	print "Processing subdir $_\n" if $debug;
	&process_dev_dir($_, 0);
	}

}

if (!$running_under_grave_robber) {
	# $verbose = $debug = 1;
	# &process_dev_dir($ARGV[0]);
	# &process_dev_dir($DEVICE_DIR);
	}

1;

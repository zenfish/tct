#!/usr/local/bin/perl

#
# given a dir, suck in all the executable files in there
#
require "realpath.pl";

sub suck_exes {
local($dir) = @_;

print "Going into suck_exes with dir $dir\n" if $verbose;

#
# we cache dirs, no sense in working too hard
#
if (defined($all_dirs_exes{$dir})) {
	print "already processed $dir\n" if $debug;
	return;
	}

$all_dirs_exes{$dir} = $dir;

# ignore proc, bad news in there...
return if ($dir =~ /^\/proc\//);

if (!opendir(DIR, $dir)) {
	warn "Can't open $dir (in suck_exes())\n";
	return;
	}

#
# look at each file in the dir; have we seen it before?
#
while (($file = readdir(DIR))) {

	print "Looking at: $dir/$file " if $debug;
	#
	# get the honest-to-god path for (symlinks) - this really slows
	# things down
	#
	$real_file = &realpath("$dir/$file") if ("$dir/$file" =~ /^\//);

	print "($real_file)\n" if $debug;

	# next if it isn't an executable file
	next unless (-x $real_file && -f $real_file);

	print "executable $dir/file - $real_file\n" if $debug;

	# collision detector
	if (defined($all_exes{$file})) {
		print "collision... $file already seen ($all_exes{$file})\n"
			if $debug;
		}
	$all_exes{$file} = $file;
	$all_location_exes{$file} = $real_file;

	# do we want this?  I think so... (for sym links)
	$all_location_exes{$file} = "$dir/$file";
	}

closedir(DIR);

}

1;

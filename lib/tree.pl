#
#  Grind through a directory; get all entries in a dir, then
# process all.  If run into dir, call itself again, ad infinitum.
# The first time this is called use a flag set to 1 to process
# the initial dir as well (instead of only subdirs).  Normally use 
# with flag==0.
#

require "crunch.pl";

sub process_dir {
local($dir, $flag) = @_;
local(@dirs);

print "going into process dir with \"$dir\"...\n" if $debug;

# others hate /proc...
next if ($proc_fs && ($dir =~ /^\/proc/));

print "processing dir $dir (in process_dir)\n" if $verbose;

if (defined($already_seen{$dir})) {
	print "We've already processed this directory ($dir), skipping...\n"
		if $debug;
	return;
	}

$already_seen{$dir} = 1;

if (!opendir(DIR, $dir)) {
	warn "Can't open $dir via opendir (in process_dir())\n" if $debug;
	return;
	}

crunch($dir) if $flag;

#
# Suck in all the dir entries
@next = readdir(DIR);

#
#  If we're stupid enough to call this with a trailing slash in the
# filename, or if we call it with just "/", strip it off.
#
$dir =~ s@/$@@;

# 
# Go over each of the dir entries
while (($_ = pop(@next))) {
	print "next dir entry: $_\n" if $debug;

	# skip ".", "..", and null dir entries
	next if ($_ eq "\." || $_ eq "\.\." || ! $_);

	print "DIR: $dir\n" if $debug;

	#
	# Add the path to the filename
	$file = $_;	# (we'll need this in a bit)
	$_ = "$dir/$_";

	#
	#  What is the entry?  Gather it for processing based on type.
	# if (-f || (-d && $recursion))    { 
	if (-f || -d) {
		print "$_ Is -f\n" if $debug;
		# get rid of extra slashes...
		$_ =~ s@/+@/@g;
		push(@files, $_);
		#
		# Config file?
		if ($_ =~ /$conf_pattern/ && $do_file_collect) {
			print "$_ Is conf file\n" if $debug;
			# $conf_files{$_} = $file;
			$conf_files{$_} = $_;
			}
		}
	if    (-f) { # dir is used above, need to use if statement
		     # to make sense of it all, else files (-f) will
		     # get confused.  Poor programming... ;-(
		   }
	elsif (-d) { print "$_ Is -d\n" if $debug; push(@dirs, $_); }
	elsif (-l) { print "$_ Is -l\n" if $debug; push(@symlinks, $_);}
	elsif (-S) { print "$_ Is -S\n" if $debug; push(@sockets, $_);}
	elsif (-p) { print "$_ Is -p\n" if $debug; push(@pipes, $_);}
	elsif (-b) { print "$_ Is -b\n" if $debug; push(@blocks, $_);}
	elsif (-c) { print "$_ Is -c\n" if $debug; push(@characters, $_);}
	else       { print "$_ Is... what the heck?\n" if $debug; }
	}

closedir(DIR);

#
#  Process subdirs in the dir (if -R flag used in mactime); crunch, and 
# search it for more dirs...
#
if ($recursion) {
	for (@dirs) {
		# some systems hate symlinks...
		next if (!$follow_dir_sym_links && -l $_);

		# others hate /proc...
		next if ($proc_fs && /^\/proc/);

		print "Processing subdir $_\n" if $debug;
		&crunch($_);
		&process_dir($_, 0);
		}
	}

#
# Crunch all the little stuff.  Still need to fix symlinks
#
while (($_=pop(@files))) { print "file $_\n" if $debug; &crunch($_); }
while (($_=pop(@symlinks)))  { print "symlink $_\n"    if $debug; &crunch($_); }
while (($_=pop(@sockets)))   { print "socket $_\n"     if $debug; &crunch($_); }
while (($_=pop(@blocks)))    { print "block file $_\n" if $debug; &crunch($_); }
while (($_=pop(@characters))){ print "character $_\n"  if $debug; &crunch($_); }
while (($_=pop(@pipes)))     { print "pipe $_\n"       if $debug; &crunch($_); }

#
# If it's a config file, grab it and put it into the vault
#
# $debug = 1;
for $file (keys %conf_files) {

	print "$file Is conf file\n" if $debug;

	next unless $file && -f $file;

	next if (!$follow_file_sym_links && -l $file);

	&vault_file_cp($file);

	($st_dev,$st_ino,$st_mode,$st_nlink,$st_uid,$st_gid,$st_rdev,$st_size,
	$st_atime,$st_mtime,$st_ctime,$st_blksize,$st_blocks) = lstat($file);

	$ls = &faux_ls($file, $st_mode);
	}

# $debug = 0;
undef(%conf_files);

}

#
#  Do some preprocessing; get forensic info on tools you're using,
# the $PATH variable, dirs you want to examine while the rest of the 
# kit is grinding away, etc.  Tell the user when you're done processing this.
#
sub do_first_looks {

print "\nStarting preprocessing paths and filenames on $hostname...\n";

die "Can't open $TCT_HOME/conf/paths.pl (in do_first_looks())!\n" unless open (PATHS, "$TCT_HOME/conf/paths.pl");

print "\tpreprocessing $TCT_HOME/conf/paths.pl\n" if $verbose;

while (<PATHS>) {
	next if (/^\s*#/ || /^\s*$/);
	# looks like...
	#	$LAST = "/bin/last";
	# or:
	#
	print "COMM: $_" if $debug;

	chop();

	($command = $_) =~ s@^.*\"(/.*)\".*$@$1@;
	print "\t==> $command\n" if $debug;

	if (!-f $command) {
		print "Can't find $command, moving on...\n" if $debug;
		next;
		}
	&crunch($command);
	}

print "finished the conf/paths.pl file...\n" if $debug;


#
#  Next process the $PATH var
#

print "Processing \$PATH elements...\n";
for (split(/:/, $ENV{'PATH'})) {
	print "$_\n";
	&process_dir($_, 0);
	}

#
#  Always start with current dir.  Nuke copying stuff temporarily
# by changing the pattern ;-)
#

$old_conf_pattern = $conf_pattern;
$conf_pattern = "XXXXXXXXXX__";

# chop($cwd = `pwd`);
chop($cwd = &command_to_string("pwd"));

print "\tProcessing dir $cwd\n";
&process_dir($cwd, 0);

# change pattern back!
$conf_pattern = $old_conf_pattern;

die "Can't open the first-looks file $toolkit (in do_first_looks())\n" 
	unless open(TOOLKIT, $toolkit);

print "\tpreprocessing $toolkit\n" if $verbose;

while (<TOOLKIT>) {
	next if (/^\s*#/ || /^\s*$/);

	chop;

	# prepend the $CORPSE value for dead stuff... normally this does 
	# nothing... testing still...
	# $_ = "$CORPSE/$_" if $CORPSE;

	# absolute path?
	if (!/^\// || !-d) { 
		print "Hey - the line \"$_\" isn't a directory or an absolute path!\n" if $debug;
		next;
		}
	print "Processing dir $_\n" if $debug;
	print "\tProcessing dir $_\n";
	&process_dir($_, 0);
	}

print "\nFinished preprocessing your toolkit... you may now use programs or\n";
print "examine files in the above directories\n\n";

}

1;

#
# usage - vault_file_cp(file2cp)
#

require "realpath.pl";

sub vault_file_cp {
local($file) = @_;
local($realfile, $symlink);

$realfile = &realpath($file);
# $realfile = &realpath($file);

print "vault cp $file ==> $realfile\n" if $debug;

$symlink = "";

# sym link?
if ($realfile ne $file) {
	$symlink = $file;
	$file = $realfile;
	}

# bad stuff... only deal with files here
return unless -f $file;

#
# we've done this elsewhere, but we're not sure who is calling this,
# so lstat the file one more time
#
($st_dev,$st_ino,$st_mode,$st_nlink,$st_uid,$st_gid,$st_rdev,$st_size,
$st_atime,$st_mtime,$st_ctime,$st_blksize,$st_blocks) = lstat($file);

if ($copy_threshold && ($st_size > $copy_threshold)) {
	warn "file ($file) is larger than the copy threshold ($copy_threshold)\n\t(in vault_file_cp())\n";
	return;
	}

&make_vault_dir($file);
&make_vault_dir($symlink) if $symlink;

#
# strip off the first slash; we'll use that for a relative
# path in the HTML file below...
#
($file2 = $file) =~ s@/@@;
# $symlink =~ s@/@@;

# $resting_place = "$DATA/$conf_vault/$path2file";

$resting_place = "$DATA/$conf_vault/$file2";

# don't need to copy it twice...
next if -f "$DATA/$conf_vault/$file2";

# don't need any dirs slipping in here... file also needs
# to exist to copy ;-)
next if -d $file || ! -f $file;

print "cp $file $resting_place\n" if $debug;
&execute_command($CP, $file, $resting_place);

if ($symlink) {
	$bare = $base = $symlink;

	#
	# target, dest => /home/usr/include/aliases.h, usr/include/aliases.h
	#
	# real                         link
	# /home/usr/include/aliases.h, /usr/include/aliases.h
	#
	#         2x for /usr/include in link
	# ln -s  ../../home/user/include/aliases.h aliases.h
	#

	print "target/base = $bare @ $base\n" if $debug;

	# rip off everything except the last bit after the last /
	$bare =~ s@^.*/([^/]*)$@$1@;
	$base =~ s@^(.*)/[^/]*$@$1@;
	$base2 = $base;

	print "target/base = $bare @ $base\n" if $debug;

	$num = ($base =~ s@/[^/]+@../@g);

	print "target/base = $bare @ $base\n" if $debug;

	print "base+dest+target = $base$dest_dir/$bare\n" if $debug;

	print "SYMLINK: $base$file ==> $bare\n" if $debug;

###	chdir("$DATA/$conf_vault/$base2");
###
###	print "CHDIR: $DATA/$conf_vault/$base2\n" if $debug;
###
###	symlink("$base$file", $bare);
###
###	chdir("$TCT_HOME");

	symlink("$base$file", "$DATA/$conf_vault/$base2/$bare");

	}

#
# update HTML file
#
die "Can't open configuration vault HTML index file $DATA/$conf_vault/index.html (in vault_file_cp())\n" unless open(CFI, ">>$DATA/$conf_vault/index.html");
 
print CFI "<a href=\"$file2\">$file</pre></a>\n";

close(CFI);

}

#
# do "mkdir -p /usr/foo/bar/baz", so we don't have to
# loop through each thing and make a dir.
#
sub make_vault_dir {
local($file) = @_;
local($dir);

print "putting $file into vault\n" if $debug;
print "putting $file into vault\n" if $verbose;

# take off leading slash and filename:
($dir) = ($file =~ m@^/(.*)/[^/]*$@);

# special case
if (!$dir) {
	$dir =~ m@/^(.*)$@;
	}

print "Making dir $dir from $file\n" if $debug;

print "mkdir $DATA/$conf_vault/$dir\n"
	if $debug;

execute_command("$MKDIR", "-p", "$DATA/$conf_vault/$dir");

}

1;

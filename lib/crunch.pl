#
#  Do a lstat and md5 on all files passed this way...
#
#  Need to handle symlinks specially...
#
#

require "realpath.pl";
require "tm_misc.pl";

sub crunch {
local($file) = @_;

# get rid of extra slashes...
$file =~ s@/+@/@g;

#
# we cache stuff, no sense in working too hard
#
if (defined($all_files_crunched{$file})) {
        print "already processed $file\n" if $debug;
        return;
        }

$all_files_crunched{$file} = $file;

return if (!$file);

print "going into crunch... $file, $do_md5\n" if $debug;

print "crunching dir $dir (in crunch)\n" if $verbose;

($st_dev,$st_ino,$st_mode,$st_nlink,$st_uid,$st_gid,$st_rdev,$st_size,
$st_atime,$st_mtime,$st_ctime,$st_blksize,$st_blocks) = lstat($file);

#
# get an MD5 of the thing we're looking at?  (Note - linux doesn't allow
# open/md5's of dirs... stupid os...)
#
if ($do_md5 && (-f $file || -d $file)) {
	# don't do named pipes or sockets, just to be safe..
	if ((!-S $file) && (! -p $file)) {
		$md5 = &md5($file);
		}
	else { $md5 = "000 $file"; }
	}
else	{ $md5 = "000 $file"; }

#
# Linux... *sigh*...
#
$st_blksize = 0 unless $st_blksize;
$st_blocks = 0 unless $st_blocks;

$ls = &faux_ls($file, $st_mode);

# ($x, $y) = split(/\s/, $md5);
($x, $y) = ($md5 =~ /(\S+)\s+(.*)$/);

# if running under mactime with the -B flag, specify body file
if ($body_out) { $tmp_body = $body; $body = $body_out; }

die "Can't open $body (in crunch())\n" unless open(BODY, ">>$body");

if ($body_out) { $body = $tmp_body; }

#
#  The select function call makes normal print's print to that file handle
# instead of STDOUT; I do this rather than write a special tm_print to make
# it print to the body.
#
select BODY;
&tm_print($x,$y,$st_dev,$st_ino,$st_mode,$ls,$st_nlink,
          $st_uid,$st_gid,$st_rdev,$st_size,$st_atime,$st_mtime,
          $st_ctime,$st_blksize,$st_blocks);
select STDOUT;

print "$md5 ||| $file Mode: $ls A: $st_atime \nM: $st_mtime \nC: $st_ctime\n" if $debug;
# print "$md5 ||| $file Mode: $ls A: $st_atime \nM: $st_mtime \nC: $st_ctime\n";

# print BODY "$md5 $st_dev,$st_ino,$st_mode,$ls,$st_nlink,$st_uid,$st_gid,$st_rdev,$st_size,$st_atime,$st_mtime,$st_ctime,$st_blksize,$st_blocks\n";

close(BODY);

#
#  Save SGID/SUID file info (no dirs) in a separate file as well -
#
if ((-u $file || -g $file) && ! -d $file) {
	die "Can't open $body.S (in crunch())\n" unless open(SBODY, ">>$body.S");
	select SBODY;
	&tm_print($x,$y,$st_dev,$st_ino,$st_mode,$ls,$st_nlink,
       		  $st_uid,$st_gid,$st_rdev,$st_size,$st_atime,$st_mtime,
       		  $st_ctime,$st_blksize,$st_blocks);
	select STDOUT;
	close(SBODY);
	}

}

sub md5 {
local($file) = @_;

# $safe_file = quotemeta($file);
# chop($md5 = `$MD5 $safe_file`);

chop($md5 = &command_to_string($MD5, $file));

# print "MD5 - $file - $md5\n";

if (!$md5 || $md5 =~ /^s*$/) { return "000 $file"; }
else { return($md5); }

}

sub faux_ls {
local($file, $mode) = @_;
local($real_file, $ls, $suid, $sgid);

#
#  What is the entry?  Gather it for processing based on type.
# 
# "d"  if file is a directory.
# "b"  if file is a block special file.
# "c"  if file is a character special file.
# "l"  if file is a symbolic link.
# "p"  if file is a named pipe (FIFO).
# "s"  if file is a socket.
# "-"  if file is a plain file.
#
# from the stat man page:
#
# S_IFSOCK   0140000   socket
# S_IFLNK    0120000   symbolic link
# S_IFREG    0100000   regular file
# S_IFBLK    0060000   block device
# S_IFDIR    0040000   directory
# S_IFCHR    0020000   character device
# S_IFIFO    0010000   fifo
# S_ISUID    0004000   set UID bit
# S_ISGID    0002000   set GID bit (see below)
# S_ISVTX    0001000   sticky bit (see below)

# default, can't figure it out ;-)
# $ls = "@";
$ls = "-";
$suid = $sgid = "";

# mostly just copied from stat.h
if    ((($mode) & 0170000) == 0100000) { $ls = "-"; }
elsif ((($mode) & 0170000) == 0040000) { $ls = "d"; }
elsif ((($mode) & 0170000) == 0120000) { $ls = "l"; }
elsif ((($mode) & 0170000) == 0060000) { $ls = "b"; } 
elsif ((($mode) & 0170000) == 0020000) { $ls = "c"; }
elsif ((($mode) & 0170000) == 0010000) { $ls = "p"; }

#
# SUID replaces first "x" with "s"
# SGID replaces second "x" with "S"
#
if    ((($mode) & 0007000) == 0004000) { $suid = "s"; }
if    ((($mode) & 0007000) == 0002000) { $sgid = "S"; }

# print "- $mode - $ls\n";
# if (-d $file) { print "DIR!  $file  $mode - $ls\n"; }

if ($mode & 000400) { $ls .= "r"; } else { $ls .= "-"; }
if ($mode & 000200) { $ls .= "w"; } else { $ls .= "-"; }
if (!$suid) {
	if ($mode & 000100) { $ls .= "x"; } else { $ls .= "-"; }
	}
else { $ls .= $suid; }

if ($mode & 000040) { $ls .= "r"; } else { $ls .= "-"; }
if ($mode & 000020) { $ls .= "w"; } else { $ls .= "-"; }
if (!$sgid) {
	if ($mode & 000010) { $ls .= "x"; } else { $ls .= "-"; }
	}
else { $ls .= $sgid; }

if ($mode & 000004) { $ls .= "r"; } else { $ls .= "-"; }
if ($mode & 000002) { $ls .= "w"; } else { $ls .= "-"; }
if ($mode & 000001) { $ls .= "x"; } else { $ls .= "-"; }

if ($file && -l $file) {
	$points_to = readlink($file);
	$ls .= " -> $points_to";
	# $real_file = &realpath($file);
	}

print "$file MODE: $mode X $ls\n" if $debug;

return $ls;

}

1;

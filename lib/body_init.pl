#
# just a little stub that puts headers in the body & body.S files
#

sub body_init {

die "Can't open $body.S (in grave_robber_init())\n" unless open(BODY, ">$body.S");
print BODY "class|host|start_time\n";
print BODY "body|$hostname|" . time . "\n";
print BODY "md5|file|st_dev|st_ino|st_mode|st_ls|st_nlink|st_uid|st_gid|st_rdev|st_size|st_atime|st_mtime|st_ctime|st_blksize|st_blocks\n";
close(BODY);

die "Can't open $body (in grave_robber_init())\n" unless open(BODY, ">$body");
print BODY "class|host|start_time\n";
print BODY "body|$hostname|" . time . "\n";
print BODY "md5|file|st_dev|st_ino|st_mode|st_ls|st_nlink|st_uid|st_gid|st_rdev|st_size|st_atime|st_mtime|st_ctime|st_blksize|st_blocks\n";
close(BODY);

}

1;


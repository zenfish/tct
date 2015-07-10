#
#  Sorta like save the whales, 'cept save the files.
# copy everything out of the "$save_these_files" config file.
#

require "vault_cp.pl";

sub process_files_to_save {

print "saving all files in $save_these_files (in &process_files_to_save())\n" if $verbose;

die "Can't open $save_these_files (in process_files_to_save())\n" unless open(STF, "$save_these_files");

# $debug = 1;

# 
# Go over each of the files
#
while (<STF>) {
	next if (/^\s*#/ || /^\s*$/);
	chop();
	print "IN FI: $_\n" if $debug;
	# print "IN FI: $_\n";

	$files = $_;

	if (!$CORPSE) { $files =~ s@\$CORPSE@@; }
	else          { $files =~ s@\$CORPSE@$CORPSE@; }

	print "next file: $files\n" if $debug;

	while (<${files}>) {
		print "Going into while...\n" if $debug;


		$file = $_;

		print "next file: $file\n" if $debug;

		&vault_file_cp($file);

		}
	}

# $debug = 0;

closedir(DIR);

}

# &process_files_to_save();

1;

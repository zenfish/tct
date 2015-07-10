#
#  Various misc functions
#

#
# Create the config vault; a location to copy all the config files, etc.
#
sub prepare_config_vault {

print "Preparing the vault...\n" if $verbose;

print "... in prepare_config_vault()\n" if $verbose;

mkdir("$DATA/$conf_vault", 0700);

#
# Toss in HTML file
#
if (-f "$DATA/$conf_vault/index.html") {unlink "$DATA/$conf_vault/index.html";}

die "Can't open configuration vault HTML index file $DATA/$conf_vault/index.html (in prepare_config_vault())\n" unless open(CFI, ">$DATA/$conf_vault/index.html");

print CFI "<HTML>\n";
print CFI "<BODY BGCOLOR=\"#FFFFFF\">\n\n";
print CFI "<h2>The Coroner's Toolkit Found these Configuration/Interesting files</h2>\n\n";
print CFI "<hr>\n\n<TT>\n";
close(CFI);

}

#
# Create the config vault; a location to copy all the config files, etc.
#
sub close_config_vault {

print "Closing the vault (in close_config_vault())\n" if $verbose;

$tmp_file = "md5_all.tmp.$$";

if (-f "$DATA/$conf_vault/index.html") {
	die "Can't open configuration vault HTML index file $DATA/$conf_vault/index.html (in close_config_vault())\n" unless open(CFI, ">>$DATA/$conf_vault/index.html");
	print CFI "</TABLE>\n\n</BODY></HTML>\n";
	close(CFI);
	}

&date_stamp("$DATA/MD5_all");

if ($opt_d) { $find = $DATA; }
else        { $find = "$DATA\_$pretty_date"; }

# want relative paths, kill off the tct_home bit if it exists
$find =~ s@$TCT_HOME/@@;

redirect_command($FIND, $find, "-print", ">$tmp_file");

print "$FIND, $find, -print  >$tmp_file\n" if $debu;

die "Can't open $tmp_file (in close_config_vault())\n" unless
	open(MD5ALL, $tmp_file);

while (<MD5ALL>) {
	chop;
	redirect_command($MD5, $_, ">>$DATA/MD5_all");
	}
close(MD5ALL);
unlink($tmp_file);

&sign_it("$DATA/MD5_all");

}

1;

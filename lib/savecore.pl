#
#    looks for files left by savecore, sets paths, etc.
#
#  Will take the first core/kernel file it finds... (don't expect it
# to work if there is more than one crash dir...!)
#

require "paths.pl";

sub setup_savecore {

print "Setting up savecore stuff (in &setup_savecore())\n" if $verbose;

print "OS: $OS\n" if $debug;

if (($OS eq "SUNOS4") || ($OS eq "SUNOS5")) {
	@foo = <$CORPSE/var/crash/*/*[0-9]>;
	for (@foo) {
		if (!$KERNEL && /unix/) {
			$KERNEL = $_ if -f;
			}
		if (!$CORE && /core/) {
			$CORE = $_ if -f;
			}
		print "file: $_ ($KERNEL/$CORE)\n" if $debug;
		}
	}

print "KERNEL & CORE files set to: $KERNEL & $CORE\n" if $verbose;
}

if (!$running_under_grave_robber) {
	require "lib/ostype.pl";
	require "lib/command.pl";
	require "lib/dig-sig.pl";

	&determine_os();

	$debug = $verbose = 1;

	&setup_savecore();
	}

1;

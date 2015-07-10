#
# Suck a self-describing table into memory.
#
# Usage: $table = &suck_table(pathname)
#
# Each table has the following components:
#
# - 'origin' is a hash table with host and time information.
#
# - 'labels' is a hash table that maps data field names to offsets 0..m
#
# - 'data' is an array of (array 0..m with data extracted from file).
#

sub suck_table {
    local($path) = @_;
    local(%table, @labels, @data, $offset, $n, $line, $ref);

    open(TABLE, "<$path") || die "Cannot open $path: $!\n";

    #
    # The file starts with origin information (host, time, etc.).
    #
    chop($line = <TABLE>);
    (@labels = &tm_split($line))
	|| die "$path: No origin dictionary record.\n";
    chop($line = <TABLE>);
    (@data = &tm_split($line))
	|| die "$path: No origin data record.\n";
    ($#labels == $#data)
	|| die "$path: Inconsistent origin field count.\n";
    for $offset (0..$#labels) {
	$table{'origin'}{@labels[$offset]} = @data[$offset];
    }

    #
    # Read the data dictionary.
    #
    chop($line = <TABLE>);
    (@labels = &tm_split($line))
	|| die "$path: No data dictionary record.\n";
    for $offset (0..$#labels) {
	$table{'labels'}{@labels[$offset]} = $offset;
    }

    #
    # Read the actual data. This is done most often so it needs to be fast.
    # Use references to avoid hash table lookups.
    #
    $ref = \@{$table{'data'}};
    for ($n = 0; chop($line = <TABLE>); $n++) {
	@{$ref->[$n]} = &tm_split($line);
	($#labels > $#{$ref->[$n]})
	    && ($#{$ref->[$n]} = $#labels);
	($#labels == $#{$ref->[$n]})
	    || die "$path: Inconsistent data field count $#labels != $#{$ref->[$n]}: $line\n";
    }

    #
    # Cleanup.
    #
    close(TABLE);
    return %table;
}

if (!$running_under_grave_robber) {
    $running_under_grave_robber = 1;
    require "tm_misc.pl";
    &suck_table($ARGV[0]);
}

1;

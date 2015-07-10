#
# print_table - print a time machine table
#
sub print_table {
    local(%table) = @_;

    #
    # The origin info.
    #
    foreach $origin (keys %{$table{'origin'}}) {
	print "origin: $origin $table{'origin'}{$origin}\n";
    }

    #
    # The data dictionary.
    #
    foreach $label (keys %{$table{'labels'}}) {
	print "offset: $label $table{'labels'}{$label}\n";
    }

    #
    # The actual data.
    #
    for $n (0..$#{$table{'data'}}) {
	print join(':', @{$table{'data'}[$n]}),"\n";
    }
}

1;

#
# tm_misc.pl - misc routines to slice and dice time machine records
#

#
# Split a time machine record.
#
sub tm_split {
        local($line) = @_;
        local(@fields);

        for (@fields = split(/\|/, $line)) {
                s/%([A-F0-9][A-F0-9])/pack("C", hex($1))/egis;
        }
        return @fields;
}

#
# tm_string - convert list to time machine record format.
#
sub tm_string {
    local(@out) = @_;

    for (@out) {
	s/([^-_`~!@#\$^&*()+={}[\]:;"'<>,.?\/a-z0-9 ])/sprintf("%%%02X",unpack("C", $1))/egis;
    }
    return join('|', @out);
}

#
# tm_print 
#
sub tm_print {
    local(@out) = @_;

    for (@out) {
	s/([^-_`~!@#\$^&*()+={}[\]:;"'<>,.?\/a-z0-9 ])/sprintf("%%%02X",unpack("C", $1))/egis;
    }
    print join('|', @out),"\n";
}

1;


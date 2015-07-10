#
# Some systems return EBUSY when grave-robbing the block device.
# Try to use the raw device instead. Provided that one exists.
#

sub rawdev
{
    my($dev) = @_;
    my($rdev);

    ($rdev = $dev) =~ s;(/dev/);$1r;;
    print "dev $dev rdev $rdev\n" unless $running_under_grave_robber;

    return (-e $rdev ? $rdev : $dev);
}

if (!$running_under_grave_robber) {
    for (@ARGV) {
	&rawdev($_);
    }
}

1;

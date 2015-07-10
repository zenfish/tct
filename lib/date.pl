#
#  A couple of date/time functions - get_date, which pretty prints the
# date, and time_stamp, which puts the current time into a file (used
# for marking all the output routines and when they were run).
#


#
# figure out the date, put it in a decent format... typically day_mon_year.
#
 
require "paths.pl";
require "datez.pl";

sub get_date 
{
    print "going into date - $DATE...\n" if $debug;

    print "Determining date (in &get_date())\n" if $verbose;

    if (!defined($pretty_date)) 
    {
	my($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst,$hr_off,$min_off)
	    = localtimez(time());
	$pretty_date = sprintf "%4d_%02d_%02d_%02d:%02d:%02d_%+03d%02d",
	    $year+1900,$mon+1,$mday,$hour,$min,$sec,$hr_off,$min_off;
    }
    return $pretty_date;
}

#
#  Put the current date into a file - truncates file, so be careful!
#
sub date_stamp {
local($file) = @_;

print "Stamping file $file with date (in &date_stamp())\n" if $verbose;

die "Can't determine the date!\n" unless -x $DATE;

&redirect_command($DATE, ">$file");

}

1;

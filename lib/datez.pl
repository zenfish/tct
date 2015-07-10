#++
# NAME
#	datez.pl 3p
# SUMMARY
#	time conversion
# SYNOPSIS
#	require 'datez.pl';
#
#	localtimez($time)
# DESCRIPTION
#	This module extends time conversion routines with explicit
#	timezone information.
#
#	localtimez() invokes the localtime() built-in primitive and
#	appends to the result list a 10th member with the UTC offset
#	in hours and an 11th member with the UTC offset in minutes.
# LICENSE
#	This software is distributed under the IBM Public License.
# AUTHOR(S)
#	Wietse Venema
#	IBM T.J. Watson Research
#	P.O. Box 704, Yorktown Heights, NY 10598, USA
#--

$MIN_SEC = 60;			# seconds in a minute
$HOUR_MIN = 60;			# minutes in an hour
$DAY_MIN = 24 * $HOUR_MIN;	# minutes in a day

# localtimez($time) - localtime() with time zone offset

sub localtimez
{
    my($time) = @_;
    my($lsec, $lmin, $lhour, $lmday, $lmon, $lyear, $lwday, $lyday, $lisdst)
	= localtime($time);
    my($gsec, $gmin, $ghour, $gmday, $gmon, $gyear, $gwday, $gyday, $gisdst)
	= gmtime($time);
    my($offset);

    # Stolen from Postfix.
    #
    # Starting with the difference in hours/minutes between 24-hour clocks,
    # adjust for differences in years, in yeardays, and in (leap) seconds.
    #
    # Assume 0..23 hours in a day, 0..59 minutes in an hour. Unfortunately
    # the localtime library spec has changed: we can no longer assume that
    # there are 0..59 seconds in a minute. It can be as many as 61 on some
    # systems, in order to account for leap seconds.

    $loffset = ($lhour - $ghour) * $HOUR_MIN + $lmin - $gmin;
    if ($lyear < $gyear) {
        $loffset -= $DAY_MIN;
    } elsif ($lyear > $gyear) {
        $loffset += $DAY_MIN;
    } elsif ($lyday < $gyday) {
        $loffset -= $DAY_MIN;
    } elsif ($lyday > $gyday) {
        $loffset += $DAY_MIN;
    }
    if ($lsec <= $gsec - $MIN_SEC) {
        $loffset -= 1;
    } elsif ($lsec >= $gsec + $MIN_SEC) {
        $loffset += 1;
    }
    return ($lsec, $lmin, $lhour, $lmday, $lmon, $lyear, $lwday, $lyday, $lisdst, $loffset / $HOUR_MIN, $loffset % $HOUR_MIN);
}

# Scaffolding...

if (!$running_under_grave_robber) {
    $verbose = 1;
    $now = time();
    my($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst,$hr_off,$min_off)
	= localtimez(time());
    printf("%4d/%02d/%02d %02d:%02d:%02d %+03d%02d\n",
	$year+1900, $mon+1, $mday, $hour, $min, $sec, $hr_off, $min_off);
}

1;

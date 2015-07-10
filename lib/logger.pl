#++
# NAME
#	logger.pl 3p
# SUMMARY
#	basic logger
# SYNOPSIS
#	require 'logger.pl';
#
#	log_init_path($path)
#
#	log_init_handle($handle)
#
#	log_item(@list)
# DESCRIPTION
#	This module provides a generic logging interface. The global
#	$logging_handle parameter specifies where logging is to be sent.
#	If the parameter is not defined, then no logging is produced.
#
#	log_init_path() initializes the logging system. All subsequent logging
#	is appended to the named file.
#
#	log_init_handle() initializes the logging system. All subsequent logging
#	is written to the named handle.
#
#	log_item() emits one line of text with a formatted time stamp and
#	whatever text is passed via the parameter list.
# LICENSE
#	This software is distributed under the IBM Public License.
# AUTHOR(S)
#	Wietse Venema
#	IBM T.J. Watson Research
#	P.O. Box 704, Yorktown Heights, NY 10598, USA
#--

# log_init_path($path) - open logfile for append mode

sub log_init_path
{
    my($path) = @_;
    my($saved_handle);
    my($username) = getpwuid($>);

    # Open for append mode, and turn on micro buffering.

    open(LOG, ">>$path") || die "Cannot open/append logfile $path: $!\n";
    print "log_init_path: opened $path\n" if $verbose;
    $saved_handle = select(LOG);
    $| = 1;
    select($saved_handle);
    log_item("logfile $path opened by", $username, "($>)");
    $logging_handle = LOG;
}

# log_init_handle($handle) - set logging handle

sub log_init_handle
{
    $logging_handle = @_[0];
    print "log_init_handle: logging handle set\n" if $verbose;
    log_item("logfile handle set by", $username, "($>)");
}

# log_item($stuff) - log time stamp and stuff

sub log_item
{
    return unless $logging_handle;

    my(@stuff) = @_;
    my($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst,$hr_off,$min_off)
        = localtimez(time());

    printf $logging_handle "%4d/%02d/%02d %02d:%02d:%02d %+03d%02d %s\n",
	$year+1900, $mon+1, $mday, $hour, $min, $sec, $hr_off, $min_off,
	join(' ', @stuff);
    printf "log_item: %s\n", join(' ', @stuff) if $verbose;
}

# Scaffolding...

if ($running_under_grave_robber) {
    require "datez.pl";
} else {
    $running_under_grave_robber = 1;
    $logger_debug = 1;
    require "../conf/coroner.cf";
    require "datez.pl";

    $verbose = 1;
    &log_init_path("/dev/tty");
    log_item("PIPEFROM_CMD", "ls", "-l");
}

1;

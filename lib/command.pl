#++
# NAME
#	command.pl 3p
# SUMMARY
#	command plumbing
# SYNOPSIS
#	require 'command.pl';
#
#	$timeouts{$command} = $short_timeout;
#
#	execute_command(@command)
#
#	pipe_command($handle, @command, $mode)
#
#	redirect_command(@command, $file)
#
#	command_to_list(@command)
#
#	command_to_string(@command)
# DESCRIPTION
#	This module attempts to provide safe alternatives for running
#	commands with or without I/O redirected to or from files or pipes,
#	and for capturing the output from a command in a variable. In
#	particular, commands are not parsed by a shell.
#
#	Each command, and its method of I/O redirection, are logged to
#	the logging module.
#
#	Each command is logged to STDOUT if $verbose is set.
#
#	Each command is subjected to a time limit as specified in the
#	%timeouts hash. The default time limit is $short_timeout. For
#	convenience, the coroner.cf file provides initial values of
#	%timeouts, $short_timeout, $med_timeout and $long_timeout.
#
#	execute_command() executes the command without performing any
#	I/O redirection. The result is the command exit status.
#
#	pipe_command() executes the command with its input or output
#	redirected to or from the specified handle. The mode argument
#	is either "|-" in order to write to the command, or "-|" in
#	order to read from the command. The result is undef in case of
#	problems, otherwise the result is the handle argument.
#
#	redirect_command() executes the command with I/O redirection
#	under control by the $file argument. In order to redirect
#	input, specify "<filename"; in order to redirect output, specify
#	">filename" or ">>filename".  The result is -1 when fork()
#	was unable to create a process, the command exit status otherwise.
#	The command exit status is also available as $?.
#
#	command_to_list() executes the command and returns a list
#	of output lines. The result is undef in case of problems.
#	The command exit status is available as $?.
#
#	command_to_string() executes the command and returns a string
#	with the command output, including newlines.  The result is undef
#	in case of problems.  The command exit status is available as $?.
# LICENSE
#	This software is distributed under the IBM Public License.
# SEE ALSO
#	timeout(1) run command with time limit
# AUTHOR(S)
#	Wietse Venema
#	IBM T.J. Watson Research
#	P.O. Box 704, Yorktown Heights, NY 10598, USA
#--

require "logger.pl";

sub timed_exec
{
    my(@command) = @_;
    my($limit);

    $limit = $short_timeout 
	unless $limit = $timeouts{$command[0]};
    return (exec($TIMEOUT, $limit, @command));
}

# pipe_command($handle, @command, $mode) - run command, return handle

sub pipe_command
{
    my($handle, @command) = @_;
    my($mode) = pop(@command);
    my($pid);

    print "pipe_command: $handle @command $mode\n"
	if $verbose;

    die "pipe_command: invalid pipe mode: $mode\n"
	unless ($mode eq "-|" || $mode eq "|-");

    &log_item($mode eq "|-" ? "PIPETO_CMD" : "PIPEFROM_CMD", @command);

    return(undef)
	unless defined($pid = open($handle, $mode));

    if ($pid == 0) {
	&timed_exec(@command) || die "cannot exec $command[0]: $!\n";
	exit 1;
    } else {
	return($handle);
    }
}

# execute_command(@command) - run command, no I/O redirection

sub execute_command
{
    my(@command) = @_;
    my($pid);

    print "execute_command: @command\n"
	if $verbose;

    &log_item("EXECUTE_CMD", @command);

    if (($pid = fork()) == 0) {
	&timed_exec(@command) || die "cannot exec $command[0]: $!\n";
	exit 1;
    } elsif ($pid == -1) {
	return(-1);
    } else {
	waitpid($pid, 0);
	return($?);
    }
}

# redirect_command(@command, $file) - run command, redirected

sub redirect_command
{
    my(@command) = @_;
    my($file) = pop(@command);
    my($pid);

    print "redirect_command: @command $file\n"
	if $verbose;

    &log_item("REDIRECT_CMD", $file, @command);

    if (($pid = fork()) == 0) {
	if ($file =~ /^>/) {
	    open(STDOUT, $file) || die "cannot open $file: $!\n";
	} elsif ($file =~ /^</) {
	    open(STDIN, $file) || die "cannot open $file: $!\n";
	} else {
	    die "redirect_command: invalid I/O redirection: $file\n";
	}
	&timed_exec(@command) || die "cannot exec $command[0]: $!\n";
	exit 1;
    } elsif ($pid == -1) {
	return(-1);
    } else {
	waitpid($pid, 0);
	return($?);
    }
}

# command_to_list(@command) - run command, output to list

sub command_to_list
{
    my(@command) = @_;
    my(@output);
    my($pid);

    print "command_to_list: @command\n"
	if $verbose;

    &log_item("PIPEFROM_CMD", @command);

    return(undef)
	unless defined($pid = open(COMMAND_TO_LIST, "-|"));

    if ($pid == 0) {
	&timed_exec(@command) || die "cannot exec $command[0]: $!\n";
	exit 1;
    } else {
	@result = <COMMAND_TO_LIST>;
	close(COMMAND_TO_LIST);
	return(@result);
    }
}

# command_to_string(@command) - run command, output to string

sub command_to_string
{
    my(@command) = @_;

    return(join('', &command_to_list(@command)));
}

1;


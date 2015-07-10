#
#  Linux version -
#
#  execute a pile of commands to suck in all sorts of information
#
#  Each command should do a date_stamp() & sign_it() to date stamp
# and md5 the results.
#

sub suck_netinfo_linux {

print "Running all sorts of network commands on host (in &suck_netinfo_linux())\n"
	if $verbose;

&netstat();
&showmount();
&arp();

&nfsstat();
&rpcinfo();
&ifconfig();

&finger();
&who();
&uptime();

&last();

sub suck_hostinfo_linux {

print "Running all sorts of commands on host (in &suck_hostinfo_linux())\n" if $verbose;

&uname();
&dmesg();
&df();

&ipcs();
&rpm();
&lsmod();

&ksyms();
&modinfo();

&top();
&lastcomm();

}

#
# network subroutines, one for each command
#

sub netstat {

if (-x $NETSTAT && !$CORPSE) {
	@n1 = ($NETSTAT, "-in", ">>$COMM_OUT/netstat-in");
	@n2 = ($NETSTAT, "-rn", ">>$COMM_OUT/netstat-rn");
	@n3 = ($NETSTAT, "-a",  "-A","inet",">>$COMM_OUT/netstat-a");
	@n4 = ($NETSTAT, "-a", "-nA", "inet", ">>$COMM_OUT/netstat-na");

	&date_stamp("$COMM_OUT/netstat-in");
	&redirect_command(@n1);
	&sign_it("$COMM_OUT/netstat-in");

	&date_stamp("$COMM_OUT/netstat-rn");
	&redirect_command(@n2);
	&sign_it("$COMM_OUT/netstat-rn");

	&date_stamp("$COMM_OUT/netstat-a");
	&redirect_command(@n3);
	&sign_it("$COMM_OUT/netstat-a");

	&date_stamp("$COMM_OUT/netstat-na");
	&redirect_command(@n4);
	&sign_it("$COMM_OUT/netstat-na");
	}
}

sub showmount {

if (-x $SHOWMOUNT && !$CORPSE) {
	&date_stamp("$COMM_OUT/showmount-e");
	&redirect_command($SHOWMOUNT, "-e", ">>$COMM_OUT/showmount-e");
	&sign_it("$COMM_OUT/showmount-e");
	}

if (-x $SHOWMOUNT && !$CORPSE) {
	&date_stamp("$COMM_OUT/showmount-a");
	&redirect_command($SHOWMOUNT, "-a", ">>$COMM_OUT/showmount-a");
	&sign_it("$COMM_OUT/showmount-a");
	}

}

sub arp {

if (-x $ARP) {
	if (!$CORPSE) {
		&date_stamp("$COMM_OUT/arp");
		&redirect_command($ARP, "-a", ">>$COMM_OUT/arp");
		&sign_it("$COMM_OUT/arp");
		}
	}

}

sub nfsstat {

if (-x $NFSSTAT) {
	if (!$CORPSE) {
		&date_stamp("$COMM_OUT/nfsstat");
		&redirect_command($NFSSTAT, "-n", ">>$COMM_OUT/nfsstat");
		&sign_it("$COMM_OUT/nfsstat");
		}
	}

}

sub rpcinfo {

if (-x $RPCINFO && !$CORPSE) {
	&date_stamp("$COMM_OUT/rpcinfo");
	&redirect_command($RPCINFO, "-p", ">>$COMM_OUT/rpcinfo");
	&sign_it("$COMM_OUT/rpcinfo");
	}
}

sub ifconfig {

if (($OS eq "SUNOS4") || ($OS eq "SUNOS5")) {
	if (-x $IFCONFIG) {
		&date_stamp("$COMM_OUT/ifconfig");
		&redirect_command($IFCONFIG, "-a", ">>$COMM_OUT/ifconfig");
		&sign_it("$COMM_OUT/ifconfig");
		}
	}

elsif (-x $IFCONFIG && -x $NETSTAT && !$CORPSE) {
	# need to figure out interfaces here
	#warn "Can't execute $NETSTAT\n" unless open(NS, "$NETSTAT -in|");
	&pipe_command(NS, $NETSTAT, "-in", "-|");

	<NS>;	# kill off the first line

	# just grab the first field, maybe duplicates
	while (<NS>) {
		($int, $x) = split(/\s/, $_);
		$net_interfaces{$int} = $int;
		}
	close(NS);

	# finally!  ;-)
	&date_stamp("$COMM_OUT/ifconfig");
	for (keys %net_interfaces) {
		&redirect_command($IFCONFIG, "$_", ">>$COMM_OUT/ifconfig");
		}
	&sign_it("$COMM_OUT/ifconfig");
	}

}

sub finger {
#
# I think -l is pretty universal...
#
if (-x $FINGER && !$CORPSE) {
	&date_stamp("$COMM_OUT/finger");
	&redirect_command($FINGER, "-l", ">>$COMM_OUT/finger");
	&sign_it("$COMM_OUT/finger");
	}

}

sub who {

if (-x $WHO && !$CORPSE) {
	&date_stamp("$COMM_OUT/who");
	&redirect_command($WHO, ">>$COMM_OUT/who");
	&sign_it("$COMM_OUT/who");
	}

if (-x $W && !$CORPSE) {
	&date_stamp("$COMM_OUT/w");
	&redirect_command($W, ">>$COMM_OUT/w");
	&sign_it("$COMM_OUT/w");
	}

}

sub uptime {

if (-x $UPTIME && !$CORPSE) {
	&date_stamp("$COMM_OUT/uptime");
	&redirect_command($UPTIME, ">>$COMM_OUT/uptime");
	&sign_it("$COMM_OUT/uptime");
	}

}

sub last {

print "Running last - this could take awhile (in &suck_hostinfo_linux())\n" if $verbose;

if (-x $LAST && !$CORPSE) {
	&date_stamp("$COMM_OUT/last");
	&redirect_command($LAST, ">>$COMM_OUT/last");
	&sign_it("$COMM_OUT/last");
	}

}

#
# need to put in a portable way of dumping bind's database... basically
# need a way to find bind's pid, send a kill -SIGINT, find the database
# that comes out, and save that.
#

}

sub uname {

if (-x $UNAME && !$CORPSE) {
	&date_stamp("$COMM_OUT/uname");
	&redirect_command("$UNAME", "-a", ">>$COMM_OUT/uname") 
	&sign_it("$COMM_OUT/uname");
	}
}

sub dmesg {

if (-x $DMESG && !$CORPSE) {
	&date_stamp("$COMM_OUT/dmesg");
	&redirect_command($DMESG, ">>$COMM_OUT/dmesg");
	&sign_it("$COMM_OUT/dmesg");
	}
}

sub df {

if (-x $DF) {
	&date_stamp("$COMM_OUT/df");
	if (!$CORPSE) {
		&redirect_command($DF, ">>$COMM_OUT/df");
		}
	else {
		&redirect_command($DF, "$CORPSE", ">>$COMM_OUT/df");
		}
	&sign_it("$COMM_OUT/df");
	}
}

sub ipcs {

if (-x $IPCS) {
	if (!$CORPSE) {
		&date_stamp("$COMM_OUT/ipcs");
		&redirect_command($IPCS, "-a", ">>$COMM_OUT/ipcs");
		&sign_it("$COMM_OUT/ipcs");
		}
	}
}

sub rpm {

if (-x $RPM ) {
	if (!$CORPSE) {
		&date_stamp("$COMM_OUT/rpm");
		&redirect_command($RPM, "-a", "-q", ">>$COMM_OUT/rpm");
		&sign_it("$COMM_OUT/rpm");
		}
	else {
		&date_stamp("$COMM_OUT/rpm");
		&redirect_command($RPM, "--root", "$CORPSE", "-a", "-q", ">>$COMM_OUT/rpm");
		&sign_it("$COMM_OUT/rpm");
		}
	}
}

sub lsmod {

if (-x $LSMOD && !$CORPSE) {
	&date_stamp("$COMM_OUT/lsmod");
	&redirect_command($LSMOD, ">>$COMM_OUT/lsmod");
	&sign_it("$COMM_OUT/lsmod");
	}
}

sub ksyms {

if (-x $KSYMS && !$CORPSE) {
	&date_stamp("$COMM_OUT/ksyms");
	&redirect_command($KSYMS, ">>$COMM_OUT/ksyms");
	&sign_it("$COMM_OUT/ksyms");
	}
}

# Kernel modules.

sub modinfo {

#
# need to cycle over lsmod, do this for each one...
#
return;

if (-x $MODINFO && !$CORPSE) {
	&date_stamp("$COMM_OUT/modinfo");
	&redirect_command($MODINFO, ">>$COMM_OUT/modinfo");
	&sign_it("$COMM_OUT/modinfo");
	}

}

#
# this could concievably go in ps_spy, but for now... "top -n 1"
# in linux seems to roughly do what "top > foo" does in other os's.
# why they changed it for linux... (#include "broken-record.dan")
#
sub top {

if (-x $TOP && !$CORPSE) {
	&date_stamp("$COMM_OUT/top");
	&redirect_command($TOP, "-b", "-n", "1", ">>$COMM_OUT/top");
	&sign_it("$COMM_OUT/top");
	}
}

sub lastcomm {

print "In lastcomm - this could take awhile (in &suck_hostinfo_linux())\n" if $verbose;

if (-x $LASTCOMM) {
	&date_stamp("$COMM_OUT/lastcomm");
	if (!$CORPSE) {
		&redirect_command($LASTCOMM, @LASTCOMM_ARGS, ">>$COMM_OUT/lastcomm");
		}
	else {
		if (! $PATH_ACCT) {
			$PATH_ACCT = "$CORPSE/var/log/pacct";
			}
		last if (!$PATH_ACCT);
		&redirect_command($LASTCOMM, @LASTCOMM_ARGS, "-f", $PATH_ACCT, ">>$COMM_OUT/lastcomm");
		}
	&sign_it("$COMM_OUT/lastcomm");
	}

}

if (!$running_under_grave_robber) {
	$running_under_grave_robber = 1;
	$debug = $verbose = 1;
	require "../conf/coroner.cf";
	require "date.pl";
	require "command.pl";
	require "dig-sig.pl";

	$COMM_OUT="/tmp/linux.pl-test";
	$OS="LINUX2";

	&suck_netinfo_linux();

	&suck_hostinfo_linux();
	}

1;


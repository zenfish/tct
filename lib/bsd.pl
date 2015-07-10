#
#  BSD version - really should have a seperate one for each flavor
# (freebsd, openbsd, bsdi, etc.), but this will do for starters...
#
#  execute a pile of commands to suck in all sorts of information
#
#  Each command should do a date_stamp() & sign_it() to date stamp
# and md5 the results.
#

sub suck_netinfo_bsd {

print "Running all sorts of network commands on host (in &suck_netinfo_bsd())\n"
	if $verbose;

if ($CORPSE && !@DEAD) { &get_bsd_corpse(); }

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

}

sub suck_hostinfo_bsd {

print "Running all sorts of commands on host (in &suck_hostinfo_bsd())\n" if $verbose;

if ($CORPSE && !@DEAD) { &get_bsd_corpse(); }

&uname();
&dmesg();
&df();

&ipcs();
&modstat();

&pstat();
&top();
&lastcomm();

}

#
# network subroutines, one for each command
#

sub netstat {

print "running netstat ($CORPSE/$KERNEL/$CORE) (in &netstat())\n" if $verbose;

if (-x $NETSTAT) {

	if (!$CORPSE) {
		@n1 = ($NETSTAT, "-in", ">>$COMM_OUT/netstat-in");
		@n2 = ($NETSTAT, "-rn", ">>$COMM_OUT/netstat-rn");
		@n3=($NETSTAT,"-a","-f","inet",">>$COMM_OUT/netstat-a");
		@n4 = ($NETSTAT, "-na", "-f", "inet",">>$COMM_OUT/netstat-na");
		}
	elsif ($KERNEL && $CORE) {
		@n1 = ($NETSTAT,"-in",@DEAD, ">>$COMM_OUT/netstat-in");
		@n2 = ($NETSTAT,"-rn",@DEAD, ">>$COMM_OUT/netstat-rn");
		@n3 = ($NETSTAT,"-a","-f","inet",@DEAD,">>$COMM_OUT/netstat-a");
		@n4 = ($NETSTAT, "-na", "-f", "inet",@DEAD,">>$COMM_OUT/netstat-na");
		}

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

if (-x $ARP && !$CORPSE) {
	&date_stamp("$COMM_OUT/arp");
	&redirect_command($ARP, "-a", ">>$COMM_OUT/arp");
	&sign_it("$COMM_OUT/arp");
	}
}

sub nfsstat {

if (-x $NFSSTAT) {
	if (!$CORPSE) {
		&date_stamp("$COMM_OUT/nfsstat");
		&redirect_command($NFSSTAT, ">>$COMM_OUT/nfsstat");
		&sign_it("$COMM_OUT/nfsstat");
	}

elsif ($KERNEL && $CORE) {
		&date_stamp("$COMM_OUT/nfsstat");
		&redirect_command($NFSSTAT, @DEAD, ">>$COMM_OUT/nfsstat");
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

if (-x $IFCONFIG && -x $NETSTAT && !$CORPSE) {
	# need to figure out interfaces here
	#warn "Can't execute $NETSTAT\n" unless open(NS, "$NETSTAT -in|");
	&pipe_command(NS, $NETSTAT, "-in", "-|");

	<NS>;	# kill off the first line

	# just grab the first field, maybe duplicates
	while (<NS>) {
		($int, $x) = split(/\s|\*/, $_);
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

print "Running last - this could take awhile (in &suck_hostinfo_bsd())\n" if $verbose;

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

sub uname {

if (-x $UNAME && !$CORPSE) {
	&date_stamp("$COMM_OUT/uname");
	&redirect_command("$UNAME", "-a", ">>$COMM_OUT/uname") 
	&sign_it("$COMM_OUT/uname");
	}
}

sub dmesg {

if (-x $DMESG) {
	if (!$CORPSE) {
		&date_stamp("$COMM_OUT/dmesg");
		&redirect_command($DMESG, ">>$COMM_OUT/dmesg");
		&sign_it("$COMM_OUT/dmesg");
		}
	else {
		&date_stamp("$COMM_OUT/dmesg");
		&redirect_command($DMESG, @DEAD, ">>$COMM_OUT/dmesg");
		&sign_it("$COMM_OUT/dmesg");
		}
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
	elsif ($KERNEL && $CORE) {
		if ($OS =~ /FREEBSD/) {
			&date_stamp("$COMM_OUT/ipcs");
			&redirect_command($IPCS, "-a","-N",$KERNEL,"-C",$CORE, ">>$COMM_OUT/ipcs");
			&sign_it("$COMM_OUT/ipcs");
			}
		}
	}
}

sub modstat {

if (-x $MODSTAT && !$CORPSE) {
	&date_stamp("$COMM_OUT/modstat");
	&redirect_command($MODSTAT, ">>$COMM_OUT/modstat");
	&sign_it("$COMM_OUT/modstat");
	}

}

sub pstat {

if (-x $PSTAT) {
	if (!$CORPSE) {
		&date_stamp("$COMM_OUT/pstat");
		&redirect_command($PSTAT, "-T", ">>$COMM_OUT/pstat");
		&sign_it("$COMM_OUT/pstat");
		}
	elsif ($KERNEL && $CORE) {
		&date_stamp("$COMM_OUT/pstat");
		&redirect_command($PSTAT, "-T", @DEAD, ">>$COMM_OUT/pstat");
		&sign_it("$COMM_OUT/pstat");
		}
	}

}

#
# this could concievably go in ps_spy, but for now... "top all", when
# redirected to a file, claims to list all processes (it doesn't seem
# to, but hey, it's better than just "top".)
#
sub top {

if (-x $TOP && !$CORPSE) {
	&date_stamp("$COMM_OUT/top");
	&redirect_command($TOP, "-b", ">>$COMM_OUT/top");
	&sign_it("$COMM_OUT/top");
	}
}

sub lastcomm {

print "Running lastcomm - this could take awhile (in &suck_hostinfo_bsd())\n" if $verbose;

if (-x $LASTCOMM) {
	&date_stamp("$COMM_OUT/lastcomm");
	if (!$CORPSE) {
		&redirect_command($LASTCOMM, @LASTCOMM_ARGS, ">>$COMM_OUT/lastcomm");
		}
	else {
		if (! $PATH_ACCT) {
			$PATH_ACCT = "$CORPSE/var/account/acct";
			}
		last if (!$PATH_ACCT);
		&redirect_command($LASTCOMM, @LASTCOMM_ARGS, "-f", $PATH_ACCT, ">>$COMM_OUT/lastcomm");
		}
	&sign_it("$COMM_OUT/lastcomm");
	}

}

#
#  find the kernel & vmcore files in a corpse
#
sub get_bsd_corpse {

print "Looking for kernel (in &get_bsd_corpse())\n" if $verbose;

# BSD saves kernel and core in /var/crash. What kernel file and
# what crash dump de we use? Give the user some control.
if (!$KERNEL) {
	$KERNEL = "$CORPSE/kernel" if -f "$CORPSE/kernel"; #FreeBSD
	$KERNEL = "$CORPSE/bsd" if -f "$CORPSE/bsd";	#BSD/OS, OpenBSD
	return unless $KERNEL;
}

if (!$CORE) {

	# this grabs the core stuff
	@foo = <$CORPSE/var/crash/vmcore.[0-9]*>;

	# this reverses the order - the highest number is the most recent
	$j = 0;
	for ($i = length(@foo)+1; $i >= 0; $i--) { $bar[$j++] = pop(@foo); }

	if ($debug) { for (@bar) { print "pop $_\n"; } }

	for (@bar) {
		print "\tlooking for $_\n" if $debug;
		$CORE = $_ if -f;
		# Was it found?  Should die on the first one in the dir...
		last if ($CORE);
	}

	return unless $CORE;
}

@DEAD = ("-N", $KERNEL, "-M", $CORE);

print "(Using kernel=$KERNEL core=$CORE)\n" if $debug;

}

if ($running_under_grave_robber) {
	$timeouts{$LASTCOMM} = $long_timeout;
} else {
	$debug = $verbose = 1;
	$running_under_grave_robber = 1;

	require "../conf/coroner.cf";
	require "logger.pl";
	require "ostype.pl";
	require "dig-sig.pl";
	require "date.pl";

	&log_init_handle(STDOUT);

	&determine_os();

	$COMM_OUT = "../data";
	# $CORPSE = "/tmp";
	# $CORE = "/dev/mem";

	&suck_netinfo_bsd();

	&suck_hostinfo_bsd();
	}

1;


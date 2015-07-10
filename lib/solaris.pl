#
#  Solaris version -
#
#  execute a pile of commands to suck in all sorts of information
#
#  Each command should do a date_stamp() & sign_it() to date stamp
# and md5 the results.
#

sub suck_netinfo_solaris {

print "Running all sorts of network commands on host (in &suck_netinfo_solaris())\n"
	if $verbose;

if ($CORPSE && !@DEAD) { &get_solaris_corpse(); }

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

sub suck_hostinfo_solaris {

print "Running all sorts of commands on host (in &suck_hostinfo_solaris())\n" if $verbose;

if ($CORPSE && !@DEAD) { &get_solaris_corpse(); }

&uname();
&dmesg();
&df();

&eeprom();
&format();
&ipcs();

&modinfo();
&modstat();
&pkginfo();

&showrev();

&swap();
&top();
&lastcomm();

}

#
# network subroutines, one for each command
#

sub netstat {

print "in netstat()\n" if $verbose;

if (-x $NETSTAT) {

	if (!$CORPSE) {
		@n1 = ($NETSTAT, "-in", ">>$COMM_OUT/netstat-in");
		@n2 = ($NETSTAT, "-rn", ">>$COMM_OUT/netstat-rn");
		@n3=($NETSTAT,"-a","-f","inet",">>$COMM_OUT/netstat-a");
		@n4 = ($NETSTAT, "-na", "-f", "inet", ">>$COMM_OUT/netstat-na");
		}

	elsif ($KERNEL && $CORE) {
		@n1 = ($NETSTAT,"-in",$KERNEL, $CORE, ">>$COMM_OUT/netstat-in");
		@n2 = ($NETSTAT,"-rn",$KERNEL, $CORE, ">>$COMM_OUT/netstat-in");
		@n3 = ($NETSTAT,"-a","-f","inet", $KERNEL, $CORE, ">>$COMM_OUT/netstat-a");
		@n4 = ($NETSTAT, "-na", "-f", "inet", $KERNEL, $CORE, ">>$COMM_OUT/netstat-na");
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

print "in showmount()\n" if $verbose;

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

print "in arp()\n" if $verbose;

if (-x $ARP) {
	if (!$CORPSE) {
		&date_stamp("$COMM_OUT/arp");
		&redirect_command($ARP, "-a", ">>$COMM_OUT/arp");
		&sign_it("$COMM_OUT/arp");
		}
	}

}

sub nfsstat {

print "in nfsstat()\n" if $verbose;

if (-x $NFSSTAT && !$CORPSE) {
	&date_stamp("$COMM_OUT/nfsstat");
	&redirect_command($NFSSTAT, ">>$COMM_OUT/nfsstat");
	&sign_it("$COMM_OUT/nfsstat");
	}

}

sub rpcinfo {

print "in rpcinfo()\n" if $verbose;

if (-x $RPCINFO && !$CORPSE) {
	&date_stamp("$COMM_OUT/rpcinfo");
	&redirect_command($RPCINFO, "-p", ">>$COMM_OUT/rpcinfo");
	&sign_it("$COMM_OUT/rpcinfo");
	}
}

sub ifconfig {

print "in ifconfig()\n" if $verbose;

if (-x $IFCONFIG && !$CORPSE) {
	&date_stamp("$COMM_OUT/ifconfig");
	&redirect_command($IFCONFIG, "-a", ">>$COMM_OUT/ifconfig");
	&sign_it("$COMM_OUT/ifconfig");
	}

}

sub finger {

print "in finger()\n" if $verbose;

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

print "in who()\n" if $verbose;

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

print "in uptime()\n" if $verbose;

if (-x $UPTIME && !$CORPSE) {
	&date_stamp("$COMM_OUT/uptime");
	&redirect_command($UPTIME, ">>$COMM_OUT/uptime");
	&sign_it("$COMM_OUT/uptime");
	}

}

sub last {

print "in last()\n" if $verbose;

print "Running last - this could take awhile (in &suck_hostinfo_solaris())\n" if $verbose;

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

print "in uname()\n" if $verbose;

if (-x $UNAME && !$CORPSE) {
	&date_stamp("$COMM_OUT/uname");
	&redirect_command("$UNAME", "-a", ">>$COMM_OUT/uname") 
	&sign_it("$COMM_OUT/uname");
	}
}

sub dmesg {

print "in dmesg()\n" if $verbose;

if (-x $DMESG && !$CORPSE) {
	&date_stamp("$COMM_OUT/dmesg");
	&redirect_command($DMESG, ">>$COMM_OUT/dmesg");
	&sign_it("$COMM_OUT/dmesg");
	}
}

sub df {

print "in df()\n" if $verbose;

#
# the useful DF... why they use the other as default is beyond me
#
if (-x $DF) {
	&date_stamp("$COMM_OUT/df2");
	if (!$CORPSE) {
		&redirect_command($DF, "-k", ">>$COMM_OUT/df2");
		}
	else {
		&redirect_command($DF, "-k", "$CORPSE", ">>$COMM_OUT/df2");
		}
	&sign_it("$COMM_OUT/df2");
	}

}

sub showrev {

print "in showrev()\n" if $verbose;

if (-x $SHOWREV && !$CORPSE) {
	&date_stamp("$COMM_OUT/showrev");
	&redirect_command($SHOWREV, "-p", ">>$COMM_OUT/showrev");
	&sign_it("$COMM_OUT/showrev");
	}

}

sub eeprom {

print "in eeprom()\n" if $verbose;

if (-x $EEPROM && !$CORPSE) {
	&date_stamp("$COMM_OUT/eeprom");
	&redirect_command($EEPROM, ">>$COMM_OUT/eeprom");
	&sign_it("$COMM_OUT/eeprom");
	}

}

sub format {

print "in format()\n" if $verbose;

if (-x $FORMAT && !$CORPSE) {
	&date_stamp("$COMM_OUT/format");
	# this should be safe... cross fingers ;-)
	&redirect_command("$FORMAT", ">>$COMM_OUT/format");
	&sign_it("$COMM_OUT/format");
	}

}

sub ipcs {

print "in ipcs()\n" if $verbose;

if (-x $IPCS) {
	if (!$CORPSE) {
		&date_stamp("$COMM_OUT/ipcs");
		&redirect_command($IPCS, "-A", ">>$COMM_OUT/ipcs");
		&sign_it("$COMM_OUT/ipcs");
		}
	elsif ($KERNEL && $CORE) {
		&date_stamp("$COMM_OUT/ipcs");
		&redirect_command($IPCS, "-A", "-N", $KERNEL, "-C", $CORE, ">>$COMM_OUT/ipcs");
		&sign_it("$COMM_OUT/ipcs");
		}
	}
}

sub swap {

print "in swap()\n" if $verbose;

if (-x $SWAP && !$CORPSE) {
	&date_stamp("$COMM_OUT/swap");
	&redirect_command($SWAP, "-l", ">>$COMM_OUT/swap");
	&sign_it("$COMM_OUT/swap");
	}

}

sub pkginfo {

print "in pkginfo()\n" if $verbose;

if (-x $PKGINFO) {
	&date_stamp("$COMM_OUT/pkginfo");
	if (!$CORPSE) {
		&redirect_command($PKGINFO, ">>$COMM_OUT/pkginfo");
		}
	else {
		&redirect_command($PKGINFO, "-d", "$CORPSE/var/sadm/pkg/", ">>$COMM_OUT/pkginfo");
		}
	&sign_it("$COMM_OUT/pkginfo");
	}

}




# Kernel modules.

sub modinfo {

print "in modinfo()\n" if $verbose;

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

sub modstat {

print "in modstat()\n" if $verbose;

if (-x $MODSTAT && !$CORPSE) {
	&date_stamp("$COMM_OUT/modstat");
	&redirect_command($MODSTAT, ">>$COMM_OUT/modstat");
	&sign_it("$COMM_OUT/modstat");
	}

}

#
# this could concievably go in ps_spy, but for now... "top all", when
# redirected to a file, claims to list all processes (it doesn't seem
# to, but hey, it's better than just "top".)
#
sub top {

print "in top()\n" if $verbose;

if (-x $TOP && !$CORPSE) {
	&date_stamp("$COMM_OUT/top");
	&redirect_command($TOP, "all", ">>$COMM_OUT/top");
	&sign_it("$COMM_OUT/top");
	}
}

sub lastcomm {

print "in lastcomm()\n" if $verbose;

print "Running lastcomm - this could take awhile (in &suck_hostinfo_solaris())\n" if $verbose;

if (-x $LASTCOMM) {
	&date_stamp("$COMM_OUT/lastcomm");
	if (!$CORPSE) {
		&redirect_command($LASTCOMM, @LASTCOMM_ARGS, ">>$COMM_OUT/lastcomm");
		}
	else {
		if (! $PATH_ACCT) {
			$PATH_ACCT = "$CORPSE/var/adm/pacct";
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
#  Basic strategy is look for things in /var/crash, if we find any
# files we work backwards (higher numbers are more recent) until we
# find a match - a kernel & corefile that have the same extension
# (*.0-99)  If there are more than 99 kernels & corefiles in there
# we have bigger problems ;-)
#
sub get_solaris_corpse {

print "Looking for kernel (in &get_solaris_corpse())\n" if $verbose;

for ($n = 99; $n >= 0; $n--) {
	$KERNEL=$CORE="";

	$KERNEL = <$CORPSE/var/crash/*/*unix*.$n>;

	# print "NEXT: $next\n" if $next;
	if (-f $KERNEL) { print "FOUND: $KERNEL\n" if $debug; }
	else { next; }

	$CORE = <$CORPSE/var/crash/*/*core*.$n>;

	if (-f $CORE) { print "FOUND: $CORE\n" if $debug; }
	else { next; }

	if ($CORE && $KERNEL) {
		print "Found em!\n" if $debug;
		last;
		}
	}

print "KERNEL & CORE files set to: $KERNEL & $CORE\n"
	if $verbose && $KERNEL && $CORE;

}

if (!$running_under_grave_robber) {
	$debug = $verbose = 1;
        $COMM_OUT = "../data";
        $CORPSE = "/tmp";
        require "../conf/coroner.cf";

	require "date.pl";
	require "command.pl";
	require "dig-sig.pl";

	&suck_netinfo_solaris();

	&suck_hostinfo_solaris();
	}

1;


#
# file: hostname.pl
# usage: $hostname = &'hostname;
#
# purpose: get hostname -- try method until we get an answer 
#       or return "Amnesiac!"
#

require 'command.pl';
 
sub hostname {
    if (!defined $hostname) {
        $hostname =  ( -x '/bin/hostname'     && &command_to_string("/bin/hostname") ) 
                  || ( -x '/usr/ucb/hostname' && &command_to_string("/usr/ucb/hostname") )
                  || ( -x '/bin/uname'        && &command_to_string("/bin/uname -n") )
                  || ( -x '/usr/bin/uuname'   && &command_to_string("/usr/bin/uuname -l"))
                  || 'Amnesiac!'; 
    }
    $hostname;
}
 
1;

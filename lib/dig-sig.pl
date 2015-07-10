#
# currently this is just an MD5 signature of a file handed to it.
# writes an output file of "filename.md5"
#

sub sign_it {
local($file) = @_;

return unless -x $MD5 && -f $file;

print "Making MD5 of file $file (in &sign_it())\n" if $verbose;

&redirect_command($MD5, $file, ">$file.md5");

}

&sign_it($ARGV[0]) unless $running_under_grave_robber;

1;

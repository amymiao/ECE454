use warnings;
use strict;

my @time;
my @cases = ("128","512","1k");
my @baseline = (3.06, 48.5, 200);
my $baseline_str = "_verify_out";

for (my $x=0; $x < scalar(@cases); $x++) {
    my $t = qx(/usr/bin/time -f "%e real" ./gol 10000 inputs/$cases[$x].pbm outputs/$cases[$x].pbm 2>&1);
    if ($t =~ m/(.*) real/) {
        $time[$x] = $1;
    }
    my $speedup = $baseline[$x] / $time[$x];
    print "$cases[$x]: Time $time[$x], Speedup $speedup\n";
    my $diff = `diff outputs/$cases[$x].pbm outputs/$cases[$x]$baseline_str.pbm`;
    if ($diff ne "") {
        print "ERROR: Diff does not match for $cases[$x]\n";
    }
}


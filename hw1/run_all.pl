use warnings;
use strict;

system("echo \"\" > junk");
my @array = ("-g","-O2","-O3","-Os","-g -pg","-g -fprofile-arcs -ftest-coverage");

print WRITE "";

for (my $x=1; $x < 7; $x++) {
    system("echo \"vpr$x ($array[$x-1])\" >> junk");
    for (my $y=0; $y < 5; $y++) {
        system("/usr/bin/time ./vpr$x iir1.map4.latren.net k4-n10.xml place.out route.out -nodisp -place_only -seed 0 2>> junk");
        system("sleep 1");
    }
    system("sed 's/user.*\$//' junk -i");
    system("sed '/^.*pagefaults.*\$/d' junk -i");
    system("echo \"\" >> junk");
}

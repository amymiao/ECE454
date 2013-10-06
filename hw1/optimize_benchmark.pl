use warnings;
use strict;

my $percent = 0;
my $sum = 0;
my $max = 5;
for (my $i=0; $i < $max; $i++) {
    `make clean; make; ./vpr iir1.map4.latren.net k4-n10.xml place.out route.out -nodisp -place_only -seed 0`;

    my $output = `gprof vpr|head -n 30|grep try_swap`;
    my @arr = split(/\s+/,$output);
    $sum += $arr[3];
    $percent += $arr[1];
    `sleep 1`;
}
my $per = $percent/$max;
my $avg = $sum/$max;
my $total = $avg/($per/100.0);
print "$avg\t$total\n";

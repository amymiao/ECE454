use strict;
use warnings;

my $str = "define NUM_FREE_LISTS";

for (my $i=1; $i < 15; $i++) {
    system("sed \"s/$str.*/$str $i/\" mm.c -i");
    system("make");
    my $out = `./mdriver|grep \"Perf index\"`;
    print "$i: $out";
}

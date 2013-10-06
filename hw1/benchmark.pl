#!/usr/bin/perl

use warnings;
use strict;

for (my $i=0; $i < 1; $i++) {
    `make clean`;
    my $time = `/usr/bin/time make`;
    open(WRITE, "> file$i.txt");
    print WRITE "Time: $time\n";
    close(WRITE);
}

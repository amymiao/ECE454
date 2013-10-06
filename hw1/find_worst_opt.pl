use warnings;
use strict;

my @options = ("-fno-auto-inc-dec ", "-fno-compare-elim ", "-fno-cprop-registers ", "-fno-dce ", "-fno-defer-pop ", "-fno-dse ", "-fno-guess-branch-probability ", "-fno-if-conversion2 ", "-fno-if-conversion ", "-fno-ipa-pure-const ", "-fno-ipa-profile ", "-fno-ipa-reference ", "-fno-merge-constants", "-fno-split-wide-types ", "-fno-tree-bit-ccp ", "-fno-tree-builtin-call-dce ", "-fno-tree-ccp ", "-fno-tree-ch ", "-fno-tree-copyrename ", "-fno-tree-dce ", "-fno-tree-dominator-opts ", "-fno-tree-dse ", "-fno-tree-forwprop ", "-fno-tree-fre ", "-fno-tree-phiprop ", "-fno-tree-sra ", "-fno-tree-pta ", "-fno-tree-ter ", "-fno-unit-at-a-time", "-fno-thread-jumps ", "-fno-align-functions  -falign-jumps ", "-fno-align-loops  -falign-labels ", "-fno-caller-saves ", "-fno-crossjumping ", "-fno-cse-follow-jumps  -fcse-skip-blocks ", "-fno-delete-null-pointer-checks ", "-fno-devirtualize", "-fno-expensive-optimizations ", "-fno-gcse  -fgcse-lm  ", "-fno-inline-small-functions ", "-fno-indirect-inlining ", "-fno-ipa-sra ", "-fno-optimize-sibling-calls ", "-fno-partial-inlining ", "-fno-peephole2 ", "-fno-regmove ", "-fno-reorder-blocks  -freorder-functions ", "-fno-rerun-cse-after-loop  ", "-fno-sched-interblock  -fsched-spec ", "-fno-schedule-insns  -fschedule-insns2 ", "-fno-strict-aliasing -fstrict-overflow ", "-fno-tree-switch-conversion -ftree-tail-merge ", "-fno-tree-pre ", "-fno-tree-vrp", "-fno-inline-functions", "-fno-unswitch-loops", "-fno-predictive-commoning", "-fno-gcse-after-reload", "-fno-tree-slp-vectorize", "-fno-vect-cost-model", "-fno-ipa-cp-clone");

my $max = 3;
my $str = join(" ",@options);

for my $opt (@options) {
    my $percent = 0;
    my $sum = 0;
    $str = "$str $opt";
    for (my $i=0; $i < $max; $i++) {
       `make clean; make OPT_FLAGS=\"-O3 -pg $str\"; ./vpr iir1.map4.latren.net k4-n10.xml place.out route.out -nodisp -place_only -seed 0`;
       my $output = `gprof vpr|head -n 30|grep try_swap`;
       my @arr = split(/\s+/,$output);
       $sum += $arr[3];
       $percent += $arr[1];
    }
    my $per = $percent/$max;
    my $avg = $sum/$max;
    my $total = $avg/($per/100.0);
    print "Try Swap: $avg\tTotal: $total\tOption: $opt\n";
}


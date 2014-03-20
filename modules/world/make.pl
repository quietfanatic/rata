#!/usr/bin/perl
use lib do {__FILE__ =~ /^(.*)[\/\\]/; ($1||'.').'/../../tool'};
use MakePl;

include '../..';

for my $mid (glob 'snd/*.mid') {
    (my $s16s = $mid) =~ s/\.mid$/.s16s/;
    rule $s16s, $mid, sub {
        run qw(timidity -idqq -s 44100 -OrSs1 -o), $s16s, $mid;
    };
}

clean_rule(sub { unlink glob 'snd/*.s16s' });

make;

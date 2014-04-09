#!/usr/bin/perl
use lib do {__FILE__ =~ /^(.*)[\/\\]/; ($1||'.').'/../../tool'};
use MakePl;

include '../..';

for my $mid (glob 'snd/*.mid') {
    (my $s16s = $mid) =~ s/\.mid$/.s16s/;
    rule $s16s, $mid, sub {
        local $SIG{__DIE__} = sub {
            unlink $s16s;
            die $_;
        };
        run qw(timidity -idqq -s 44100 -OrSs1 -o), $s16s, $mid;
    };
}
rule 'snd/shoot.s16s', [], sub {
    die "Unfortunately, you're missing a file because quietfanatic was in a hurry to make something presentable by Wednesday.\n";
};

clean_rule(sub { unlink glob 'snd/*.s16s' });

make;

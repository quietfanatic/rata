#!/usr/bin/perl
use strict;
use warnings;
use autodie;

open my $T, '>', "t/$ARGV[0]-$ARGV[1].t";
print $T "#!/usr/bin/perl\nsystem qw(./rata --test $ARGV[1])";
close $T;

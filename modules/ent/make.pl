#!/usr/bin/perl
use lib do {__FILE__ =~ /^(.*)[\/\\]/; ($1||'.').'/tool'};
use MakePl;

include '../..';

mkdir 'tmp';

objects(qw(various bipeds control));

clean_rule(glob 'tmp/*');

make;

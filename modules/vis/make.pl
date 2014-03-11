#!/usr/bin/perl
use lib do {__FILE__ =~ /^(.*)[\/\\]/; ($1||'.').'/../../tool'};
use MakePl;

include '../..';

mkdir 'tmp';

objects(qw(common color images light text tiles models));

clean_rule(glob 'tmp/*');

make;

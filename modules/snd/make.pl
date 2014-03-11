#!/usr/bin/perl
use lib do {__FILE__ =~ /^(.*)[\/\\]/; ($1||'.').'/../../tool'};
use MakePl;

mkdir 'tmp';

objects(qw(audio));

clean_rule(glob 'tmp/*');

make;

#!/usr/bin/perl
use lib do {__FILE__ =~ /^(.*)[\/\\]/; ($1||'.').'/../../tool'};
use MakePl;

include '../..';

mkdir 'tmp';

objects(qw(tap main));
cppc_rule('tmp/tap.t.o', 'src/tap.t.cpp');
ld_rule('tmp/t', ['tmp/tap.o', 'tmp/tap.t.o', 'tmp/main.o']);

test_rule('tmp/t');
clean_rule(glob 'tmp/*');
defaults 'test';

make;

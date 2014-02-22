#!/usr/bin/perl
use lib do {__FILE__ =~ /^(.*)[\/\\]/; ($1||'.').'/tool'};
use MakePl;

include '../..';

mkdir 'tmp';

objects(qw(commands opengl window));

clean_rule(glob 'tmp/*');

make;

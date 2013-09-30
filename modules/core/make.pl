#!/usr/bin/perl
use lib do {__FILE__ =~ /^(.*)[\/\\]/; ($1||'.').'/tool'};
use MakePl;

include '../..';

objects(qw(commands input opengl window));

clean_rule(glob 'tmp/*');

make;

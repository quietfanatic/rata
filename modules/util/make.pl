#!/usr/bin/perl
use lib do {__FILE__ =~ /^(.*)[\/\\]/; ($1||'.').'/tool'};
use MakePl;

include '../..';

objects(qw(debug geometry integration));

clean_rule(glob 'tmp/*');

defaults('test');

make;

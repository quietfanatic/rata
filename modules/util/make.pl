#!/usr/bin/perl

use strict;
use warnings;
use FindBin;
use lib "$FindBin::Bin/../../tool";
use MakePl;

include '../..';

objects(qw(debug geometry));

clean_rule(glob 'tmp/*');

defaults('test');

make;

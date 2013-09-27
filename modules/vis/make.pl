#!/usr/bin/perl

use strict;
use warnings;
use FindBin;
use lib "$FindBin::Bin/../../tool";
use MakePl;

include '../..';

objects(qw(common color images text tiles models));

clean_rule(glob 'tmp/*');

make;

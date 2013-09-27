#!/usr/bin/perl

use strict;
use warnings;
use FindBin;
use lib "$FindBin::Bin/../../tool";
use MakePl;

include '../..';

objects(qw(rooms tiles camera));

clean_rule(glob 'tmp/*');

make;

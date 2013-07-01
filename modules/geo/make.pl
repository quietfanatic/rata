#!/usr/bin/perl

use strict;
use warnings;
use FindBin;
use if !$^S, lib => "$FindBin::Bin/../../tool";
use Make_pl;

workflow {

    include '../..';

    cppc_rule('tmp/rooms.o', 'src/rooms.cpp');
    cppc_rule('tmp/tiles.o', 'src/tiles.cpp');

    clean_rule(glob 'tmp/*');

};




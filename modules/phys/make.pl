#!/usr/bin/perl

use strict;
use warnings;
use FindBin;
use if !$^S, lib => "$FindBin::Bin/../../tool";
use Make_pl;

workflow {

    include '../..';

    cppc_rule('tmp/phys.o', 'src/phys.cpp');
    cppc_rule('tmp/aux.o', 'src/aux.cpp');
    cppc_rule('tmp/ground.o', 'src/ground.cpp');

    clean_rule(glob 'tmp/*');

};




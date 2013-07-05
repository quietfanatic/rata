#!/usr/bin/perl

use strict;
use warnings;
use FindBin;
use if !$^S, lib => "$FindBin::Bin/../../tool";
use Make_pl;

workflow {

    include '../..';

    cppc_rule('tmp/sprites.o', 'src/sprites.cpp');
    cppc_rule('tmp/models.o', 'src/models.cpp');
    cppc_rule('tmp/graffiti.o', 'src/graffiti.cpp');
    cppc_rule('tmp/text.o', 'src/text.cpp');

    clean_rule(glob 'tmp/*');

};




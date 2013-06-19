#!/usr/bin/perl

use strict;
use warnings;
use FindBin;
use if !%::make, lib => "$FindBin::Bin/../../tool/make.pl";
use make;

workflow {

    include '../..';

    cppc_rule('tmp/phys.o', 'src/phys.cpp');
    cppc_rule('tmp/aux.o', 'src/aux.cpp');
    cppc_rule('tmp/ground.o', 'src/ground.cpp');

    clean_rule(glob 'tmp/*');

};




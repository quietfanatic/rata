#!/usr/bin/perl

use strict;
use warnings;
use FindBin;
use if !%make::, lib => "$FindBin::Bin/../../tool";
use make;

workflow {

    include '../..';

    cppc_rule('tmp/debug.o', 'src/debug.cpp');
    cppc_rule('tmp/math.o', 'src/math.cpp');

    clean_rule(glob 'tmp/*');

};




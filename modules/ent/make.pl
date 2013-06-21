#!/usr/bin/perl

use strict;
use warnings;
use FindBin;
use if !%make::, lib => "$FindBin::Bin/../../tool";
use make;

workflow {

    include '../..';
    
    cppc_rule('tmp/various.o', 'src/various.cpp');
    cppc_rule('tmp/humans.o', 'src/humans.cpp');

    clean_rule(glob 'tmp/*');

};




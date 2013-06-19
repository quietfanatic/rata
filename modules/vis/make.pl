#!/usr/bin/perl

use strict;
use warnings;
use FindBin;
use if !%::make, lib => "$FindBin::Bin/../../tool/make.pl";
use make;

workflow {

    include '../..';

    cppc_rule('tmp/sprites.o', 'src/sprites.cpp');
    cppc_rule('tmp/models.o', 'src/models.cpp');
    cppc_rule('tmp/shaders.o', 'src/shaders.cpp');
    cppc_rule('tmp/graffiti.o', 'src/graffiti.cpp');
    cppc_rule('tmp/text.o', 'src/text.cpp');
    cppc_rule('tmp/console.o', 'src/console.cpp');

    clean_rule(glob 'tmp/*');

};




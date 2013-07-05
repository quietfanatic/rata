#!/usr/bin/perl

use strict;
use warnings;
use FindBin;
use if !$^S, lib => "$FindBin::Bin/../../tool";
use Make_pl;


workflow {

    include '../..';

    cppc_rule('tmp/game.o', 'src/game.cpp');
    cppc_rule('tmp/phases.o', 'src/phases.cpp');
    cppc_rule('tmp/state.o', 'src/state.cpp');
    cppc_rule('tmp/input.o', 'src/input.cpp');
    cppc_rule('tmp/images.o', 'src/images.cpp');
    cppc_rule('tmp/opengl.o', 'src/opengl.cpp');

    clean_rule(glob 'tmp/*');

};




#!/usr/bin/perl

use strict;
use warnings;
use FindBin;
use if !%make::, lib => "$FindBin::Bin/../../tool";
use make;

workflow {

    our @objects = qw<
        tmp/main.o
        ../core/tmp/game.o
        ../core/tmp/phases.o
        ../core/tmp/commands.o
        ../core/tmp/state.o
        ../core/tmp/input.o
        ../core/tmp/images.o
        ../core/tmp/opengl.o
        ../ent/tmp/humans.o
        ../ent/tmp/various.o
        ../geo/tmp/rooms.o
        ../geo/tmp/tiles.o
        ../hacc/tmp/haccable_standard.o
        ../hacc/tmp/strings.o
        ../hacc/tmp/files.o
        ../hacc/tmp/hacc.o
        ../hacc/tmp/haccable.o
        ../hacc/tmp/haccable_files.o
        ../phys/tmp/phys.o
        ../phys/tmp/aux.o
        ../phys/tmp/ground.o
        ../vis/tmp/sprites.o
        ../vis/tmp/shaders.o
        ../vis/tmp/models.o
        ../vis/tmp/graffiti.o
        ../vis/tmp/text.o
        ../vis/tmp/console.o
        ../util/tmp/debug.o
        ../util/tmp/math.o
    >;
    our @libs = qw(-lgc -lGL -lglfw -lSOIL ../../lib/libBox2D.a);

    include '../..';

    cppc_rule('tmp/main.o', 'src/main.cpp');

     # Here's the main program
    ld_rule('../../rata', [@objects], [@libs]);

    clean_rule(glob 'tmp/*');

};




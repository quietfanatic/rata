#!/usr/bin/perl

use strict;
use warnings;
BEGIN {
    unless (defined $FindBin::Bin) {
        require FindBin;
        require "$FindBin::Bin/../../tool/make.pm";
    }
    make->import(':all');
}
use autodie qw<:all>;
use File::Path qw<remove_tree>;


workflow {

    our @objects = qw<
        tmp/main.o
        tmp/rata.o
        ../core/tmp/game.o
        ../core/tmp/commands.o
        ../core/tmp/resources.o 
        ../core/tmp/state.o
        ../core/tmp/input.o
        ../hacc/tmp/haccable_standard.o
        ../hacc/tmp/strings.o
        ../hacc/tmp/files.o
        ../hacc/tmp/hacc.o
        ../hacc/tmp/haccable.o
        ../hacc/tmp/haccable_files.o
        ../phys/tmp/phys.o
        ../phys/tmp/aux.o
        ../phys/tmp/ground.o
        ../vis/tmp/images.o
        ../vis/tmp/sprites.o
        ../vis/tmp/shaders.o
        ../vis/tmp/models.o
        ../util/tmp/debug.o
        ../ent/tmp/various.o
    >;
    our @libs = qw(-lgc -lGL -lglfw -lSOIL ../../lib/libBox2D.a);

    include '../..';

    subdep('src/main.cpp', [qw<../core/inc/state.h>]);
    subdep('src/rata.cpp', [qw<../hacc/inc/everything.h ../phys/inc/phys.h ../vis/inc/models.h ../vis/inc/sprites.h ../core/inc/state.h>]);

    cppc_rule('tmp/main.o', 'src/main.cpp');
    cppc_rule('tmp/rata.o', 'src/rata.cpp');

     # Here's the main program
    ld_rule('../../rata', [@objects], [@libs]);

    clean_rule(glob 'tmp/*');

};




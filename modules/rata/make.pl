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
        ../phys/tmp/phys.o
        ../vis/tmp/vis.o
    >;
    our @libs = qw(-lGL -lglfw -lSOIL ../../lib/libBox2D.a);

    include '../..';

    subdep('src/main.cpp', [qw<../core/inc/game.h ../core/inc/state.h ../vis/inc/vis.h ../hacc/inc/everything.h ../phys/inc/phys.h>]);

    cppc_rule('tmp/main.o', 'src/main.cpp');

     # Here's the main program
    rule '../../rata', [@objects], sub {
        ld @objects, @libs, output '../../rata'
    };

    clean_rule(glob 'tmp/*');

};




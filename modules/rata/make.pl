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
        tmp/main.o ../core/tmp/loop.o ../vis/tmp/vis.o
        ../core/tmp/commands.o ../hacc/tmp/hacc.o ../hacc/tmp/haccable.o
        ../hacc/tmp/haccable_standard.o ../hacc/tmp/strings.o ../hacc/tmp/files.o
        ../core/tmp/resources.o ../core/tmp/state.o
    >;
    our @libs = qw(-lGL -lglfw -lSOIL ../../lib/libBox2D.a);

    include '../..';

    subdep('src/main.cpp', '../core/inc/loop.h');

    cppc_rule('tmp/main.o', 'src/main.cpp');

     # Here's the main program
    rule '../../rata', [@objects], sub {
        ld @objects, @libs, output '../../rata'
    };

    clean_rule(glob 'tmp/*');

};




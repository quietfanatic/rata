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

    include '../..';

    subdep('inc/images.h', [qw<../core/inc/resources.h ../util/inc/Vec.h ../util/inc/Rect.h>]);
    subdep('inc/models.h', [qw<inc/images.h ../util/inc/Vec.h ../hacc/inc/haccable_pointers.h>]);
    subdep('inc/sprites.h', [qw<inc/images.h ../util/inc/organization.h>]);
    subdep('inc/shaders.h', [qw<../core/inc/resources.h>]);
    subdep('inc/graffiti.h', [qw<../util/inc/Vec.h>]);

    subdep('src/images.cpp', [qw<inc/images.h ../hacc/inc/everything.h>]);
    subdep('src/images.cpp', [qw<inc/sprites.h ../hacc/inc/everything.h ../core/inc/game.h ../util/inc/debug.h inc/shaders.h>]);
    subdep('src/models.cpp', [qw<inc/models.h ../hacc/inc/everything.h inc/sprites.h ../core/inc/game.h ../core/inc/commands.h>]);
    subdep('src/shaders.cpp', [qw<../hacc/inc/everything.h inc/shaders.h>]);
    subdep('src/graffiti.cpp', [qw<inc/graffiti.h ../hacc/inc/everything.h ../core/inc/game.h>]);

    cppc_rule('tmp/images.o', 'src/images.cpp');
    cppc_rule('tmp/sprites.o', 'src/sprites.cpp');
    cppc_rule('tmp/models.o', 'src/models.cpp');
    cppc_rule('tmp/shaders.o', 'src/shaders.cpp');
    cppc_rule('tmp/graffiti.o', 'src/graffiti.cpp');

    clean_rule(glob 'tmp/*');

};




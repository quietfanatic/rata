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

    subdep('inc/rooms.h', [qw<../util/inc/Vec.h ../util/inc/Rect.h ../util/inc/organization.h>]);
    subdep('inc/tiles.h', [qw<inc/rooms.h ../phys/inc/phys.h ../vis/inc/images.h>]);
    subdep('src/rooms.cpp', [qw<inc/rooms.h ../util/inc/debug.h ../core/inc/game.h ../hacc/inc/everything.h>]);
    subdep('src/tiles.cpp', [qw<inc/tiles.h ../util/inc/math.h ../hacc/inc/everything.h ../util/inc/debug.h ../vis/inc/shaders.h ../core/inc/game.h>]);
    cppc_rule('tmp/rooms.o', 'src/rooms.cpp');
    cppc_rule('tmp/tiles.o', 'src/tiles.cpp');

    clean_rule(glob 'tmp/*');

};




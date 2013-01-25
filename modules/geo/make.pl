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
    subdep('src/rooms.cpp', [qw<inc/rooms.h ../util/inc/debug.h ../core/inc/game.h ../hacc/inc/everything.h>]);
    cppc_rule('tmp/rooms.o', 'src/rooms.cpp');

    clean_rule(glob 'tmp/*');

};




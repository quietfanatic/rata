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

    subdep('inc/vis.h', [qw<../core/inc/resources.h ../util/inc/Vec.h ../util/inc/Rect.h ../util/inc/organization.h>]);
    subdep('inc/models.h', [qw<inc/vis.h ../core/inc/resources.h ../util/inc/Vec.h ../hacc/inc/haccable_pointers.h>]);
    subdep('src/vis.cpp', [qw<inc/vis.h ../hacc/inc/everything.h ../core/inc/game.h ../util/inc/debug.h>]);
    subdep('src/modules.cpp', [qw<inc/models.h ../hacc/inc/everything.h ../inc/vis.h>]);

    cppc_rule('tmp/vis.o', 'src/vis.cpp');
    cppc_rule('tmp/models.o', 'src/models.cpp');

    clean_rule(glob 'tmp/*');

};




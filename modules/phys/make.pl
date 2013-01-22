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

    subdep('inc/phys.h', [qw<../util/inc/Vec.h ../util/inc/organization.h ../hacc/inc/haccable_integration.h>]);
    subdep('inc/aux.h', 'inc/phys.h');
    subdep('inc/ground.h', 'inc/phys.h');
    subdep('src/phys.cpp', [qw<inc/phys.h ../core/inc/game.h ../hacc/inc/haccable_standard.h ../hacc/inc/haccable_pointers.h ../util/inc/debug.h>]);
    subdep('src/aux.cpp', 'inc/aux.h');
    subdep('src/ground.cpp', 'inc/ground.h');
    cppc_rule('tmp/phys.o', 'src/phys.cpp');
    cppc_rule('tmp/aux.o', 'src/aux.cpp');
    cppc_rule('tmp/ground.o', 'src/ground.cpp');

    clean_rule(glob 'tmp/*');

};




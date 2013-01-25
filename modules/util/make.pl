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

    subdep('inc/Vec.h', [qw<../hacc/inc/haccable.h inc/honestly.h>]);
    subdep('src/organization.h', [qw<../hacc/inc/haccable.h inc/honestly.h>]);
    subdep('src/debug.cpp', [qw<inc/debug.h ../hacc/inc/haccable.h ../core/inc/commands.h ../hacc/inc/haccable_pointers.h>]);
    subdep('src/math.cpp', [qw<inc/math.h inc/honestly.h>]);

    cppc_rule('tmp/debug.o', 'src/debug.cpp');
    cppc_rule('tmp/math.o', 'src/math.cpp');

    clean_rule(glob 'tmp/*');

};




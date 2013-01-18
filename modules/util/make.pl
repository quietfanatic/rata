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

    cppc_rule('tmp/debug.o', 'src/debug.cpp');

    clean_rule(glob 'tmp/*');

};




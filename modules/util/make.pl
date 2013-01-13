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

    subdep('inc/Vec.h', '../hacc/inc/haccable.h');
    subdep('src/debug.cpp', 'inc/debug.h');

    cppc_rule('tmp/debug.o', 'src/debug.cpp');

    clean_rule(glob 'tmp/*');

};




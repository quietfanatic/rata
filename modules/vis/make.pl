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

    subdep('inc/vis.h', [qw<../core/inc/resources.h ../util/inc/Vec.h ../util/inc/Rect.h>]);
    subdep('src/vis.cpp', [qw<inc/vis.h ../hacc/inc/everything.h>]);

    cppc_rule('tmp/vis.o', 'src/vis.cpp');

    clean_rule(glob 'tmp/*');

};




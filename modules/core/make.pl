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

    subdep('src/loop.cpp', [qw<inc/loop.h ../vis/inc/vis.h>]);
    subdep('src/commands.cpp', [qw<inc/commands.h ../hacc/inc/everything.h>]);
    subdep('inc/resources.h', '../util/inc/honestly.h');
    subdep('src/resources.cpp', 'inc/resources.h');

    cppc_rule('tmp/loop.o', 'src/loop.cpp');
    cppc_rule('tmp/commands.o', 'src/commands.cpp');
    cppc_rule('tmp/resources.o', 'src/resources.cpp');

    clean_rule(glob 'tmp/*');

};




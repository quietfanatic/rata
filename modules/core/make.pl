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

    subdep('src/loop.cpp', 'inc/loop.h');
    subdep('src/loop.cpp', '../vis/inc/vis.h');
    subdep('src/commands.cpp', 'inc/commands.h');

    cppc_rule('tmp/loop.o', 'src/loop.cpp');
    cppc_rule('tmp/commands.o', 'src/commands.cpp');

    clean_rule(glob 'tmp/*');

};




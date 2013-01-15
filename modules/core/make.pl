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

    subdep('src/loop.cpp', [qw<inc/loop.h>]);
    subdep('src/commands.cpp', [qw<inc/commands.h ../hacc/inc/everything.h>]);
    subdep('inc/resources.h', '../util/inc/honestly.h');
    subdep('src/resources.cpp', 'inc/resources.h');
    subdep('inc/state.h', '../util/inc/organization.h');
    subdep('src/state.cpp', [qw<inc/state.h ../hacc/inc/haccable_pointers.h>]);

    cppc_rule('tmp/loop.o', 'src/loop.cpp');
    cppc_rule('tmp/commands.o', 'src/commands.cpp');
    cppc_rule('tmp/resources.o', 'src/resources.cpp');
    cppc_rule('tmp/state.o', 'src/state.cpp');

    clean_rule(glob 'tmp/*');

};




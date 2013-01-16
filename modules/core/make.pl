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

    subdep('src/game.cpp', [qw<inc/game.h inc/input.h>]);
    subdep('src/commands.cpp', [qw<inc/commands.h ../hacc/inc/everything.h>]);
    subdep('inc/resources.h', '../util/inc/honestly.h');
    subdep('src/resources.cpp', 'inc/resources.h');
    subdep('inc/state.h', '../util/inc/organization.h');
    subdep('src/state.cpp', [qw<inc/state.h inc/game.h ../hacc/inc/everything.h inc/commands.h>]);
    subdep('src/input.h', '../util/inc/honestly.h');
    subdep('src/input.cpp', [qw<inc/input.h inc/game.h inc/commands.h>]);

    cppc_rule('tmp/game.o', 'src/game.cpp');
    cppc_rule('tmp/commands.o', 'src/commands.cpp');
    cppc_rule('tmp/resources.o', 'src/resources.cpp');
    cppc_rule('tmp/state.o', 'src/state.cpp');
    cppc_rule('tmp/input.o', 'src/input.cpp');

    clean_rule(glob 'tmp/*');

};




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

    cppc_rule('tmp/game.o', 'src/game.cpp');
    cppc_rule('tmp/phases.o', 'src/phases.cpp');
    cppc_rule('tmp/state.o', 'src/state.cpp');
    cppc_rule('tmp/commands.o', 'src/commands.cpp');
    cppc_rule('tmp/resources.o', 'src/resources.cpp');
    cppc_rule('tmp/input.o', 'src/input.cpp');

    clean_rule(glob 'tmp/*');

};




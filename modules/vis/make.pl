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

    subdep('src/vis.cpp', 'inc/vis.h');

    cppc_rule('tmp/vis.o', 'src/vis.cpp');

    clean_rule(glob 'tmp/*');

};




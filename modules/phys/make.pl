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

    subdep('src/phys.cpp', 'inc/phys.h');
    cppc_rule('tmp/phys.o', 'src/phys.cpp');

    clean_rule(glob 'tmp/*');

};




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

    cppc_rule('tmp/phys.o', 'src/phys.cpp');
    cppc_rule('tmp/aux.o', 'src/aux.cpp');
    cppc_rule('tmp/ground.o', 'src/ground.cpp');

    clean_rule(glob 'tmp/*');

};




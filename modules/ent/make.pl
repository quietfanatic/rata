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
    
    subdep('src/various.cpp', [qw<../phys/inc/phys.h ../vis/inc/sprites.h ../hacc/inc/everything.h>]);
    cppc_rule('tmp/various.o', 'src/various.cpp');

    clean_rule(glob 'tmp/*');

};




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
    
    subdep('inc/humans.h', [qw<../phys/inc/phys.h ../phys/inc/aux.h ../phys/inc/ground.h ../geo/inc/rooms.h ../vis/inc/models.h>]);
    subdep('src/humans.cpp', [qw<inc/humans.h ../hacc/inc/everything.h>]);
    subdep('src/various.cpp', [qw<../phys/inc/phys.h ../vis/inc/sprites.h ../hacc/inc/everything.h>]);
    cppc_rule('tmp/various.o', 'src/various.cpp');
    cppc_rule('tmp/humans.o', 'src/humans.cpp');

    clean_rule(glob 'tmp/*');

};




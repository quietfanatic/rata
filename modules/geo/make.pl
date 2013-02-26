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

    cppc_rule('tmp/rooms.o', 'src/rooms.cpp');
    cppc_rule('tmp/tiles.o', 'src/tiles.cpp');

    clean_rule(glob 'tmp/*');

};




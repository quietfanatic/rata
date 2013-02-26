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
    
    cppc_rule('tmp/various.o', 'src/various.cpp');
    cppc_rule('tmp/humans.o', 'src/humans.cpp');

    clean_rule(glob 'tmp/*');

};




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


workflow {

    include '../..';
    
    
    cppc_rule('tmp/tap.o', ['src/tap.cpp', 'inc/tap.h']);
    cppc_rule('tmp/t.o', ['src/tap.t.cpp', 'inc/tap.h']);
    cppc_rule('tmp/tap_make_test_main.o', ['src/tap_make_test_main.cpp', 'inc/tap.h']);

    ld_rule('tmp/t', ['tmp/tap.o', 'tmp/t.o', 'tmp/tap_make_test_main.o']);

    test_rule('tmp/t');
    
    clean_rule(glob 'tmp/*');

    defaults 'test';

};




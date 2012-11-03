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
    
    rules [
        ['tmp/tap.o', ['src/tap.cpp', 'include/tap.h']],
        ['tmp/tap_testmain.o', ['src/tap_testmain.cpp', 'include/tap.h']],
    ], sub {
        cppc($_[1][0], output($_[0][0]))
    };

    rule 'tmp/tap_testmain', ['tmp/tap.o', 'tmp/tap_testmain.o'], sub {
        ld('tmp/tap.o', 'tmp/tap_testmain.o', output('tmp/tap_testmain'));
    };

    phony 'test', 'tmp/tap_testmain', sub {
        run "tmp/tap_testmain --test | prove -e '' -";
    };

    phony 'clean', [], sub {
        no autodie;
        remove_tree glob 'tmp/*';
    };

    defaults 'test';

};




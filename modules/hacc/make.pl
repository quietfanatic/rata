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

    cppc_rule('tmp/hacc.o', ['src/hacc.cpp', 'inc/hacc.h']);
    cppc_rule('tmp/strings.o', ['src/strings.cpp', 'inc/strings.h', 'inc/hacc.h']);
    cppc_rule('tmp/haccable.o', ['src/haccable.cpp', 'inc/haccable.h', 'inc/strings.h', 'inc/hacc.h']);
    cppc_rule('tmp/haccable_standard.o', ['src/haccable_standard.cpp', 'inc/haccable_standard.h', 'inc/haccable.h', 'inc/strings.h', 'inc/hacc.h']);
    cppc_rule('tmp/hacc.t.o', ['test/hacc.t.cpp', 'inc/hacc.h']);
    cppc_rule('tmp/strings.t.o', ['test/strings.t.cpp', 'inc/strings.h', 'inc/hacc.h']);
    cppc_rule('tmp/haccable.t.o', ['test/haccable.t.cpp', 'inc/haccable.h', 'inc/strings.h', 'inc/hacc.h']);
    cppc_rule('tmp/haccable_standard.t.o', ['test/haccable_standard.t.cpp', 'inc/haccable_standard.h', 'inc/haccable.h', 'inc/strings.h', 'inc/hacc.h']);
    ld_rule('tmp/t', [targetmatch(qr/^tmp\/[^\/]*\.o$/), '../tap/tmp/tap.o', '../tap/tmp/tap_make_test_main.o']);

    test_rule('tmp/t');

    clean_rule(glob 'tmp/*');

    defaults 'test';

};




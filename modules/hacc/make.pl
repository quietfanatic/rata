#!/usr/bin/perl

use strict;
use warnings;
use FindBin;
use if !%make::, lib => "$FindBin::Bin/../../tool";
use make qw<:all>;
use autodie qw<:all>;
use File::Path qw<remove_tree>;


workflow {

    include '../..';

    cppc_rule('tmp/hacc.o', 'src/hacc.cpp');
    cppc_rule('tmp/strings.o', 'src/strings.cpp');
    cppc_rule('tmp/hacc.t.o', 'test/hacc.t.cpp');
    cppc_rule('tmp/strings.t.o', 'test/strings.t.cpp');

    ld_rule('tmp/t', [qw[
        tmp/hacc.o tmp/strings.o tmp/hacc.t.o tmp/strings.t.o
        ../tap/tmp/tap.o ../tap/tmp/tap_make_test_main.o
    ]]);

    test_rule('tmp/t');

    clean_rule(glob 'tmp/*');

    defaults 'test';

};




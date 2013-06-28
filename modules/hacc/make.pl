#!/usr/bin/perl

use strict;
use warnings;
use FindBin;
use if !%make::, lib => "$FindBin::Bin/../../tool";
use make;

workflow {

    include '../..';

    cppc_rule('tmp/tree.o', 'src/tree.cpp');
    cppc_rule('tmp/strings.o', 'src/strings.cpp');
    cppc_rule('tmp/dynamicism.o', 'src/dynamicism.cpp');
    cppc_rule('tmp/haccable.o', 'src/haccable.cpp');
    cppc_rule('tmp/files.o', 'src/files.cpp');
    cppc_rule('tmp/tree.t.o', 'test/tree.t.cpp');
    cppc_rule('tmp/strings.t.o', 'test/strings.t.cpp');
    cppc_rule('tmp/haccable.t.o', 'test/haccable.t.cpp');
    cppc_rule('tmp/files.t.o', 'test/files.t.cpp');

    ld_rule('tmp/t', [qw[
        tmp/tree.o tmp/tree.t.o
        tmp/strings.o tmp/strings.t.o
        tmp/dynamicism.o
        tmp/haccable.o tmp/haccable.t.o
        tmp/files.o tmp/files.t.o
        ../tap/tmp/tap.o ../tap/tmp/tap_make_test_main.o
    ]]);

    test_rule('tmp/t');

    clean_rule(glob 'tmp/*');

    defaults 'test';

};




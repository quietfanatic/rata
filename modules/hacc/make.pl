#!/usr/bin/perl

use strict;
use warnings;
use FindBin;
use if !$^S, lib => "$FindBin::Bin/../../tool";
use Make_pl;

workflow {

    include '../..';

    objects(qw(common tree strings types dynamicism haccable files haccable_standard));

    cppc_rule('tmp/tree.t.o', 'test/tree.t.cpp');
    cppc_rule('tmp/strings.t.o', 'test/strings.t.cpp');
    cppc_rule('tmp/haccable.t.o', 'test/haccable.t.cpp');
    cppc_rule('tmp/files.t.o', 'test/files.t.cpp');

    ld_rule('tmp/t', [qw[
        tmp/common.o
        tmp/tree.o tmp/tree.t.o
        tmp/strings.o tmp/strings.t.o
        tmp/types.o
        tmp/dynamicism.o
        tmp/haccable.o tmp/haccable.t.o
        tmp/haccable_standard.o
        tmp/files.o tmp/files.t.o
        ../tap/tmp/tap.o ../tap/tmp/tap_make_test_main.o
    ]]);

    test_rule('tmp/t');

    clean_rule(glob 'tmp/*');

    defaults 'test';

};




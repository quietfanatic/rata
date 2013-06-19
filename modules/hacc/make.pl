#!/usr/bin/perl

use strict;
use warnings;
use FindBin;
use if !%::make, lib => "$FindBin::Bin/../../tool/make.pl";
use make;

workflow {

    include '../..';

    cppc_rule('tmp/hacc.o', 'src/hacc.cpp');
    cppc_rule('tmp/strings.o', 'src/strings.cpp');
    cppc_rule('tmp/files.o', 'src/files.cpp');
    cppc_rule('tmp/haccable.o', 'src/haccable.cpp');
    cppc_rule('tmp/haccable_standard.o', 'src/haccable_standard.cpp');
    cppc_rule('tmp/haccable_files.o', 'src/haccable_files.cpp');
    cppc_rule('tmp/hacc.t.o', 'test/hacc.t.cpp');
    cppc_rule('tmp/strings.t.o', 'test/strings.t.cpp');
    cppc_rule('tmp/files.t.o', 'test/files.t.cpp');
    cppc_rule('tmp/haccable.t.o', 'test/haccable.t.cpp');
    cppc_rule('tmp/everything.t.o', 'test/everything.t.cpp');
    cppc_rule('tmp/haccable_standard.t.o', 'test/haccable_standard.t.cpp');
    cppc_rule('tmp/haccable_pointers.t.o', 'test/haccable_pointers.t.cpp');
    cppc_rule('tmp/haccable_integration.t.o', 'test/haccable_integration.t.cpp');
    cppc_rule('tmp/haccable_files.t.o', 'test/haccable_files.t.cpp');
    ld_rule('tmp/t', [targetmatch(qr/^tmp\/[^\/]*\.o$/), '../tap/tmp/tap.o', '../tap/tmp/tap_make_test_main.o'], '-lgc');

    test_rule('tmp/t');

    clean_rule(glob 'tmp/*');

    defaults 'test';

};




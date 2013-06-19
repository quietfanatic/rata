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

    cppc_rule('tmp/debug.o', 'src/debug.cpp');
    cppc_rule('tmp/math.o', 'src/math.cpp');
    cppc_rule('tmp/annotations.o', 'src/annotations.cpp');
    cppc_rule('tmp/annotations.t.o', 'test/annotations.t.cpp');
    ld_rule('tmp/t', [qw[tmp/annotations.o tmp/annotations.t.o ../tap/tmp/tap.o ../tap/tmp/tap_make_test_main.o]]);
    test_rule('tmp/t');

    clean_rule(glob 'tmp/*');
    
    defaults('test');

};




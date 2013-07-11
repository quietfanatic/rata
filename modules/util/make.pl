#!/usr/bin/perl

use strict;
use warnings;
use FindBin;
use if !$^S, lib => "$FindBin::Bin/../../tool";
use Make_pl;

workflow {

    include '../..';

    objects(qw(debug geometry annotations));

    ld_rule('tmp/t', [qw[tmp/annotations.o tmp/annotations.t.o ../tap/tmp/tap.o ../tap/tmp/tap_make_test_main.o]]);
    test_rule('tmp/t');

    clean_rule(glob 'tmp/*');
    
    defaults('test');

};




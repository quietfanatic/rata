#!/usr/bin/perl

use strict;
use warnings;
use FindBin;
use if !$^S, lib => "$FindBin::Bin/../../tool";
use Make_pl;

workflow {

    include '../..';

    objects(qw(debug geometry));

    clean_rule(glob 'tmp/*');
    
    defaults('test');

};




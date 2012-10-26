#!/usr/bin/perl

use strict;
use warnings;
use lib 'tool';
use make;
use autodie;

rule 'a', 'b', sub {
    system 'cp', 'b', 'a';
};
rule 'b', 'c', sub {
    system 'cp', 'c', 'b';
};
rule 'c', 'a', sub {
    system 'cp', 'a', 'c';
};

exit(!make(@ARGV));



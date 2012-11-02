#!/usr/bin/perl

use strict;
use warnings;
use lib 'tool';
use make qw(rule phony make_argv);
use autodie qw(:all);


my @compile = qw(g++-4.7 -std=c++11 -fmax-errors=5 -c);
my @link = qw(g++-4.7);
my @devel_flags = qw(-Wall -ggdb);
my @libs = qw();
my $run_testmain = './tap_testmain';
sub output { '-o', $_[0] }

my %modules = (
    'tap' => [qw(tap.h)],
    'tap_testmain' => [qw(tap.h)],
);
my @objects = map "$_.o", keys %modules;

sub compile { my @args = @_; system @compile, @args }
sub _link { my @args = @_; system @link, @args }


phony 'test', 'tap_testmain', sub {
    system "$run_testmain --test | prove -e '' -";
};

for my $m (keys %modules) {
    rule "$m.o", ["$m.cpp", @{$modules{$m}}], sub {
        compile("$m.cpp", @devel_flags, output("$m.o"))
    };
}

rule 'tap_testmain', \@objects, sub {
    _link @objects, output('tap_testmain');
};

phony 'clean', [], sub {
    no autodie;
    unlink @objects;
    unlink 'tap_testmain';
};


make_argv;



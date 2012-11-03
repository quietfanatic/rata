#!/usr/bin/perl

use strict;
use warnings;
use lib 'tool';
use make qw(workflow rule phony run);
use autodie qw(:all);


my @compile = qw(g++-4.7 -std=c++11 -fmax-errors=5 -c);
my @link = qw(g++-4.7);
my @devel_flags = qw(-Wall -ggdb);
my @libs = qw();
sub execute { "./$_[0]" }
sub output { '-o', $_[0] }

my %modules = (
    'tap' => [qw(tap.h)],
    'tap_testmain' => [qw(tap.h)],
);
my %executables = (
    'tap_testmain' => [qw(tap.o tap_testmain.o)],
);

my @objects = map "$_.o", keys %modules;

sub compile { my @args = @_; run @compile, @args }
sub _link { my @args = @_; run @link, @args }

workflow {

    phony 'test', 'tap_testmain', sub {
        run execute('tap_testmain') . " --test | prove -e '' -";
    };

    for my $module (keys %modules) {
        rule "$module.o", ["$module.cpp", @{$modules{$module}}], sub {
            compile "$module.cpp", @devel_flags, output("$module.o")
        };
    }
    
    for my $exe (keys %executables) {
        rule $exe, $executables{$exe}, sub {
            _link @{$executables{$exe}}, output($exe);
        };
    }

    phony 'clean', [], sub {
        no autodie;
        unlink map "$_.o", keys %modules;
        unlink keys %executables;
    };

};




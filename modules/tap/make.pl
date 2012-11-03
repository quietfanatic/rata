#!/usr/bin/perl

use strict;
use warnings;
BEGIN { unless ($^S) { require FindBin; require lib; lib->import("$FindBin::Bin/../../tool"); } }
use make qw<workflow rule phony defaults run>;
use autodie qw<:all>;
use File::Path qw<remove_tree>;


my @compile = qw<g++-4.7 -std=c++11 -fmax-errors=5 -c>;
my @link = qw<g++-4.7>;
my @devel_flags = qw<-Wall -ggdb>;
my @libs = qw<>;
sub execute { $_[0] =~ /\// ? $_[0] : "./$_[0]" }
sub output { '-o', $_[0] }

sub compile { my @args = @_; run @compile, @args }
sub _link { my @args = @_; run @link, @args }


workflow {

    my %modules = (
        tap => ['include/tap.h'],
        tap_testmain => ['include/tap.h'],
    );

    for my $module (keys %modules) {
        rule "tmp/$module.o", ["src/$module.cpp", @{$modules{$module}}], sub {
            compile "src/$module.cpp", @devel_flags, output("tmp/$module.o")
        };
    }
    rule 'tmp/tap_testmain', ['tmp/tap.o', 'tmp/tap_testmain.o'], sub {
        _link 'tmp/tap.o', 'tmp/tap_testmain.o', output('tmp/tap_testmain');
    };

    phony 'test', 'tmp/tap_testmain', sub {
        run execute('tmp/tap_testmain') . " --test | prove -e '' -";
    };

    phony 'clean', [], sub {
        no autodie;
        remove_tree glob 'tmp/*';
    };

    defaults 'test';

};




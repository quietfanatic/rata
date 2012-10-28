#!/usr/bin/perl

use strict;
use warnings;
use lib 'tool';
use make qw(rule phony make_argv);
use autodie qw(:all);


my $cppc = 'g++-4.7';
my @cppc_flags = qw(-std=c++11 -fmax-errors=5);
my @devel_flags = qw(-Wall -Wno-format-security -Wno-unused-label -ggdb);
my @libs = qw(-lGL -lglfw -lSOIL lib/libBox2D.a);
my @cppc_output_flag = qw(-o);

my $main_program = 'rata';
my $main_cpp = 'src/main.cpp';
my @all_cpps = glob 'src/*.cpp src/*.h src/*.hpp';


phony 'test', $main_program, sub {
    system 'prove', '-f';
};

rule $main_program, \@all_cpps, sub {
    system $cppc, @cppc_flags, @devel_flags, $main_cpp, @libs, @cppc_output_flag, $main_program;
};

phony 'clean', [], sub {
    no autodie;
    unlink $main_program;
};


make_argv;



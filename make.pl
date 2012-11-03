#!/usr/bin/perl

use strict;
use warnings;
use lib 'tool';
use make qw(workflow rule phony defaults);
use autodie qw(:all);


my $cppc = 'g++-4.7';
my $compile = '-c';
my @cppc_flags = qw(-std=c++11 -fmax-errors=5);
my @devel_flags = qw(-Wall -Wno-format-security -Wno-unused-label -ggdb);
my @libs = qw(-lGL -lglfw -lSOIL lib/libBox2D.a);
my @cppc_output_flag = qw(-o);

my $main_program = 'rata';
my $main_cpp = 'src/main.cpp';
my @all_cpps = glob 'src/*.cpp src/*.cpp src/*.h src/*.hpp';
my @all_hacc_cpps = glob 'src/hacc/*.cpp';
my @all_hacc_headers = glob 'src/hacc/*.h';
my @all_hacc_objects;

phony 'test', $main_program, sub {
    system 'prove', '-f';
};

for (@all_hacc_cpps) {
    my $cpp = $_;
    (my $object = $cpp) =~ s/\.cpp$/.o/;
    push @all_hacc_objects, $object;
    rule $object, [$cpp, @all_hacc_headers], sub {
        system $cppc, $compile, @cppc_flags, @devel_flags, $cpp, @cppc_output_flag, $object;
    }
}

rule $main_program, [@all_cpps, @all_hacc_objects], sub {
    system $cppc, @cppc_flags, @devel_flags, $main_cpp, @libs, @cppc_output_flag, $main_program;
};

phony 'clean', [], sub {
    no autodie;
    unlink $main_program;
};


make_argv;



#!/usr/bin/perl

use strict;
use warnings;
use feature 'switch';

BEGIN {
	require 'tool/maker.pl';
	require 'tool/epl.pl';
}


my $cpp = 'g++-4.7';
my @cpp_flags = qw(-std=c++11 -fmax-errors=10);
my @test_flags = qw(-Wall -O1 -ggdb);
my @prof_flags = qw(-O1 -pg);
my @release_flags = qw(-O3);
my @libs = qw(-lGL -lglfw -lSOIL);

my @allepls = glob 'src/*/*.epl src/*.epl';
my @allcpps = glob 'src/*/*.cpp src/*.cpp';
my $maincpp = 'src/rata.cpp';
my @allactorcpps = map { /^(src\/actor\/.*\.c\+\+)\.epl$/; $1 } glob 'src/actor/*.cpp.epl';
my @allautocpps = grep { -e "$_.epl" } glob 'src/*/*.cpp src/*.cpp';
my @allxcfs = glob 'src/xcf/*.xcf';
my $mainprogram = 'built/rata';

our $force = 0;
our $v = 0;
sub make;
sub make {
	for (@_) {
		when ('-v') {
			$v = 1;
		}
		when ('--force') {
			$force = 1;
		}
		when ('all') {
			make 'epls';
			dependsys $mainprogram, \@allcpps,
				$cpp, @cpp_flags, @test_flags, $maincpp, @libs, '-o', $mainprogram;
		}
		when('epls') {
			make @allepls;
		}
		when ('xcfs') {
			depend [glob 'tmp/img tmp/img/*.png'], [@allxcfs], sub { make @allxcfs };
		}
		when ('clean') {
			undependcmd \@allautocpps, 'unlink', @allautocpps;
			undependcmd 'tmp/img', 'remove', 'tmp/img';
			undependcmd 'built/img', 'remove', 'built/img';
			undependcmd $mainprogram, 'remove', $mainprogram;
		}
		when ('src/imgs.cpp.epl') {
			depend 'src/imgs.cpp', ['tool/combine.pl', 'src/imgs.cpp.epl', @allxcfs], sub {
				make 'xcfs';
				makecmd 'mkdir', 'built/img';
				makecmd 'eplf', 'src/imgs.cpp', 'src/imgs.cpp.epl'
			};
		}
		when (/^(.*)\.epl$/) {
			my ($to, $from) = ($1, $_);
			my $deps = $1 eq 'src/Actor.c++' ? [$from, @allactorcpps, 'tool/epl.pl'] : [$from, 'tool/epl.pl'];
			dependcmd $to, $deps, 'eplf', $to, $from;
		}
		when (/^(.*)\.xcf$/) {
			dependcmd 'tmp/img', [], 'mkdir', 'tmp/img';
			makesys qw(gimp-2.8 --no-data --no-fonts --no-interface -b),
				"(load \"tool/lw-export-layers.scm\") (lw-export-layers-cmd \"$_\" \"tmp/img\") (gimp-quit 0)";
		}
		default {
			die "$0: No rule for target: $_\n";
		}
	}
}

if (defined $^S and $^S == 0) {
	make(@ARGV ? @ARGV : 'all');
}


1;

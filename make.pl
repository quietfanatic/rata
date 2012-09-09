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
my @test_flags = qw(-O1 -ggdb);
my @prof_flags = qw(-O1 -pg);
my @release_flags = qw(-O3);
my @libs = qw(-lGL -lglfw -lSOIL);

my @allcpp = (glob('actor/*.cpp'), glob('*.cpp'));
my @allactorcpps = map { /^(actor\/.*\.c\+\+)\.epl$/; $1 } glob('actor/*.cpp.epl');
my @allxcfnames = map { /^xcf\/(.*)\.xcf/; $1 } glob('xcf/*.xcf');

sub actor_info {
	our $EPL_IN_FILENAME =~ /^actor\/(\d+)-(\w+)\.c\+\+\.epl$/ or die "$EPL_IN_FILENAME: actor_info called in a non-actor file\n";
	my ($num, $name) = ($1, $2);
	<<END
	uint id () { return $1; }
	CStr name () { return "$2"; }
END
}


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
			dependsys 'rata', \@allcpp,
				$cpp, @cpp_flags, @test_flags, 'rata.cpp', @libs, '-o', 'rata';
		}
		when('epls') {
			make glob 'actor/*.epl';
			make glob '*.epl';
		}
		when ('xcfs') {
			depend ['tmpimg', glob 'tmpimg/*.png'], [glob 'xcf/*.xcf'], sub { make glob 'xcf/*.xcf' };
		}
		when ('clean') {
			my @gencpps = grep { -e "$_.epl" } glob('actor/*.cpp'), glob('*.cpp');
			undependcmd \@gencpps, 'unlink', @gencpps;
			undependcmd 'tmpimg', 'remove', 'tmpimg';
			undependcmd 'img', 'remove', 'img';
			undependcmd 'rata', 'remove', 'rata';
		}
		when ('imgs.cpp.epl') {
			depend 'imgs.cpp', ['tool/combine.pl', 'imgs.cpp.epl', glob('xcf/*.xcf')], sub {
				make 'xcfs';
				makecmd 'mkdir', 'img';
				makecmd 'eplf', 'imgs.cpp', 'imgs.cpp.epl'
			};
		}
		when (/^(.*)\.epl$/) {
			my ($to, $from) = ($1, $_);
			my $deps = $1 eq 'Actor.c++' ? [$from, @allactorcpps, 'tool/epl.pl'] : [$from, 'tool/epl.pl'];
			dependcmd $to, $deps, 'eplf', $to, $from;
		}
		when (/^(.*)\.xcf$/) {
			dependcmd 'tmpimg', [], 'mkdir', 'tmpimg';
			makesys qw(gimp-2.8 --no-data --no-fonts --no-interface -b),
				"(load \"tool/lw-export-layers.scm\") (lw-export-layers-cmd \"$_\" \"tmpimg\") (gimp-quit 0)";
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

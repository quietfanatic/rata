#!/usr/bin/perl

use strict;
use warnings;
use feature 'switch';
use autodie;

BEGIN {
	require 'tool/maker.pl';
	require 'tool/epl.pl';
}


my $cpp = 'g++-4.7';
my @cpp_flags = qw(-std=c++11 -fmax-errors=10);
my @test_flags = qw(-Wall -O1 -ggdb);
my @prof_flags = qw(-O1 -pg);
my @release_flags = qw(-O3);
my @libs = qw(-lGL -lglfw -lSOIL built/lib/libBox2D.a);

sub epl_to_cpp {
    $_[0] =~ /^src\/(.*)\.epl/;
    return "tmp/$1";
}
sub cpp_to_epl {
    $_[0] =~ /^tmp\/(.*)\.cpp/;
    return "src/$1.cpp.epl";
}

my @allepls = glob 'src/*/*.epl src/*.epl';
my @allcpps = glob 'src/*/*.cpp src/*.cpp';
my @alltestcpps = glob 't/*.t.cpp';
my @alltests = glob 't/*.t';
my $maincpp = 'src/rata.cpp';
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
            make 'test';
        }
        when ('test') {
            make 'program';
            make 'testwrappers';
            makesys 'prove', @alltests;
        }
		when ('program') {
			make 'epls';
	        makecmd 'mkdir', 'built';
			dependsys $mainprogram, \@allcpps,
				$cpp, @cpp_flags, @test_flags, $maincpp, @libs, '-o', $mainprogram;
		}
        when ('testwrappers') {
            make @alltestcpps;
        }
		when ('epls') {
            makecmd 'mkdir', 'tmp';
            makecmd 'mkdir', 'tmp/actor';
			make @allepls;
		}
		when ('xcfs') {
			depend [glob 'tmp/img tmp/img/*.png'], [@allxcfs], sub { make @allxcfs };
		}
		when ('clean') {
			undependcmd 'tmp', 'remove', 'tmp';
			undependcmd 'built/img', 'remove', 'built/img';
			undependcmd $mainprogram, 'remove', $mainprogram;
		}
		when ('src/imgs.cpp.epl') {
			my ($to, $from) = (epl_to_cpp($_), $_);
			depend $to, [$from, 'tool/combine.pl', @allxcfs], sub {
				make 'xcfs';
				makecmd 'mkdir', 'built';
				makecmd 'mkdir', 'built/img';
				makecmd 'eplf', $to, $from;
			};
		}
        when (/^(t\/(.*)\.t)\.cpp/) {
            my ($to, $name) = ($1, $2);
            depend $to, [], sub {
                open my $TW, '>', $to;
                print $TW "system '$mainprogram', '--test', '$name';";
                close $TW;
            }
        }
		when (/\.epl$/) {
			my ($to, $from) = (epl_to_cpp($_), $_);
			dependcmd $to, [$from, 'tool/epl.pl'], 'eplf', $to, $from;
		}
		when (/\.xcf$/) {
            makecmd 'mkdir', 'tmp';
			makecmd 'mkdir', 'tmp/img';
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

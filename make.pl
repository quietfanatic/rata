#!/usr/bin/perl

use strict;
use warnings;
use feature 'switch';

do 'epl.pl';

my $force = 0;
sub depend {
	my ($to, $from, $command) = @_;
	if ($force) {
		$command->();
		return;
	}
	my $tom = 99**999;
	my $fromm = -(99**999);
	for (ref $to eq 'ARRAY' ? @$to : $to) {
		if (!-e $_) {
			$command->();
			return;
		}
		my $m = -(-M $_);
		$tom = $m if $m < $tom;
	}
	for (ref $from eq 'ARRAY' ? @$from : $from) {
		my $m = -(-M $_);
		$fromm = $m if $m > $fromm;
	}
	if ($fromm > $tom) {
		$command->();
	}
}

sub makesys {
	print "@_\n";
	my $err = system(@_);
	if ($err) {
		die "$0: Stopping due to error code $err\n";
	};
}
sub makecmd {
	my ($s, @args) = @_;
	print "$s( ", (join ', ', @args), " );\n";
	no strict 'refs';
	return &{"$s"}(@args);
}

my @allcpp = glob('actor/*.c++'), glob('*.c++');

sub make {
	for (@_) {
		when ('--force') {
			$force = 1;
		}
		when ('all') {
			make('epls');
			depend('rata', \@allcpp, sub{
				makesys(qw(g++-4.7 -std=c++11 -O1 rata.c++ -o rata));
			});
		}
		when('epls') {
			make(glob("actor/*.epl"));
			make(glob("*.epl"));
		}
		when('clean') {
			for (glob('actor/*.c++'), glob('*.c++')) {
				if (-e "$_.epl") {
					print "unlink('$_');\n";
					unlink($_);
				}
			}
		}
		when(/^(.*)\.epl$/) {
			my ($to, $from) = ($1, $_);
			depend($to, $from, sub{ makecmd 'eplf', $from, $to });
		}
		default {
			die "$0: No rule for target: $_\n";
		}
	}
}

if (defined $^S and $^S == 0) {
	make(@ARGV);
}




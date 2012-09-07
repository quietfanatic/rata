#!/usr/bin/perl

use strict;
use warnings;
use feature 'switch';

do 'epl.pl';

my $force = 0;
my $v = 0;
sub depend {
	my ($to, $from, $command) = @_;
	my @to = ref $to eq 'ARRAY' ? @$to : $to;
	my @from = ref $from eq 'ARRAY' ? @$from : $from;
	if ($v) {
		print "@to <- @from\n\t";
	}
	if ($force) {
		$command->();
		return;
	}
	my $tom = 99**999;
	my $fromm = -(99**999);
	for (@to) {
		if (!-e $_) {
			$command->();
			return;
		}
		my $m = -(-M $_);
		$tom = $m if $m < $tom;
	}
	for (@from) {
		my $m = -(-M $_);
		$fromm = $m if $m > $fromm;
	}
	if ($fromm > $tom) {
		$command->();
	}
	elsif ($v) {
		print "# no need\n";
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
	if (defined &{"$s"}) {
		return &{"$s"}(@args);
	}
	else {  # builtin ops aren't routines :(
		my $r = eval "$s(\@args)";
		if ($@) {
			die $@;
		}
		return $r;
	}
}

my @allcpp = (glob('actor/*.c++'), glob('*.c++'));

sub make {
	for (@_) {
		when ('-v') {
			$v = 1;
		}
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
					makecmd 'unlink', $_;
				}
			}
			makecmd 'unlink', 'rata';
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
	make(@ARGV ? @ARGV : 'all');
}




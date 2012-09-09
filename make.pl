#!/usr/bin/perl

use strict;
use warnings;
use feature 'switch';

BEGIN { require 'tool/epl.pl' }

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
		my $r = eval (@args == 1 ? "$s(\$args[0])" : "$s(\@args)");
		if ($@) {
			die $@;
		}
		return $r;
	}
}

my @allcpp = (glob('actor/*.c++'), glob('*.c++'));
my @allactorcpps = map { /^(actor\/.*\.c\+\+)\.epl$/; $1 } glob('actor/*.c++.epl');
my @allxcfnames = map { /^xcf\/(.*)\.xcf/; $1 } glob('xcf/*.xcf');

sub actor_info {
	our $EPL_IN_FILENAME =~ /^actor\/(\d+)-(\w+)\.c\+\+\.epl$/ or die "$EPL_IN_FILENAME: actor_info called in a non-actor file\n";
	my ($num, $name) = ($1, $2);
	<<END
	uint id () { return $1; }
	CStr name () { return "$2"; }
END
}

sub remove;
sub remove {
	if (-d $_[0]) {
		remove $_ for glob "$_[0]/*";
		rmdir $_[0];
	}
	else {
		unlink $_[0];
	}
}

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
			depend 'rata', \@allcpp, sub {
				makesys qw(g++-4.7 -std=c++11 -fmax-errors=10 -O1 rata.c++ -o rata);
			};
		}
		when('epls') {
			make glob 'actor/*.epl';
			make glob '*.epl';
		}
		when ('xcfs') {
			make glob 'xcf/*.xcf';
		}
		when('clean') {
			makecmd 'unlink', grep { -e "$_.epl" } glob('actor/*.c++'), glob('*.c++');
			makecmd 'remove', 'tmpimgs';
			makecmd 'remove', 'img';
		}
		when('imgs.c++.epl') {
			depend 'imgs.c++', ['imgs.c++.epl', glob('xcf/*.xcf')], sub {
				make 'xcfs';
				makecmd 'mkdir', 'img';
				makecmd 'eplf', 'imgs.c++', 'imgs.c++.epl'
			};
			makecmd 'remove', 'tmpimgs';
		}
		when(/^(.*)\.epl$/) {
			my ($to, $from) = ($1, $_);
			my $deps = $1 eq 'Actor.c++' ? [$from, @allactorcpps] : $from;
			depend $to, $deps, sub{ makecmd 'eplf', $to, $from };
		}
		when (/^(.*)\.xcf$/) {
			depend 'tmpimgs', [], sub { makecmd 'mkdir', 'tmpimgs' };
			makesys qw(gimp-2.8 --no-data --no-fonts --no-interface -b),
				"(load \"tool/lw-export-layers.scm\") (lw-export-layers-cmd \"$_\" \"tmpimgs\") (gimp-quit 0)";
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

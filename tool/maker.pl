#!/usr/bin/perl

use strict;
use warnings;
use feature 'switch';

our $force;
our $v;

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


sub undepend {
	my ($to, $command) = @_;
	my @to = ref $to eq 'ARRAY' ? @$to : $to;
	if ($v) {
		print "@to !<-\n\t";
	}
	if ($force or grep { -e $_ } @to) {
		$command->();
		return;
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
sub dependcmd {
	my ($to, $from, @cmd) = @_;
	depend $to, $from, sub { makecmd @cmd };
}
sub dependsys {
	my ($to, $from, @cmd) = @_;
	depend $to, $from, sub { makesys @cmd };
}
sub undependcmd {
	my ($to, @cmd) = @_;
	undepend $to, sub { makecmd @cmd };
}
sub undependsys {
	my ($to, @cmd) = @_;
	undepend $to, sub { makecmd @cmd };
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

if (defined $^S and $^S == 0) {
	make(@ARGV ? @ARGV : 'all');
}


1;

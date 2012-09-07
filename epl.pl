#!/usr/bin/perl

use strict;
use warnings;

my $EPL_IN_FILENAME = "(unknown file)";
my $EPL_OUT_FILENAME = "(unknown file)";

sub epl {
	my ($IN) = @_;
	my $code;

	while (length $IN and $IN =~ s/^(.*?)<%//s) {
		(my $text = $1) =~ s/('|\\)/\\$1/sg;
		$code .= "\$OUT.='$text';";
		if (length $IN and $IN =~ s/^(.*?)%>//s) {
			my $perl = $1;
			if ($perl =~ s/^=//s) {
				$code .= "\$OUT.=do{$perl};";
			}
			else {
				$code .= "$perl;";
			}
		}
	};
	$IN =~ s/('|\\)/\\$1/sg;
	$code .= "\$OUT.='$IN';";
	
	my $OUT = '';
	eval $code;
	if ($@) {
		die $@;
	}
	return $OUT;
}

sub eplf {
	my ($inf, $outf) = @_;
	open my $INF, '<', $inf or die "$0: Could not open $inf for reading: $!\n";
	open my $OUTF, '>', $outf or die "$0: Could not open $outf for writing: $!\n";
	local $/;
	$EPL_IN_FILENAME = $inf;
	$EPL_OUT_FILENAME = $outf;
	print $OUTF epl(readline($INF));
	$EPL_IN_FILENAME = '(unknown file)';
	$EPL_OUT_FILENAME = '(unknown file)';
	close $INF;
	close $OUTF;
}

if (defined $^S and $^S == 0) {
	local $/;
	print(epl(<>));
}



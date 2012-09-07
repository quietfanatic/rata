#!/usr/bin/perl

use strict;
use warnings;



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
#	my $in = <$INF>;
#	print $in;
	print $OUTF epl(readline($INF));
	close $INF;
	close $OUTF;
}

if (defined $^S and $^S == 0) {
	local $/;
	print(epl(<>));
}



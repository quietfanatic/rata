
use strict;
use warnings;
#use Regexp::Debugger;
use Carp;

our %images;  # our for access in epl


for (glob 'tmp/img/*.xcf-*.png') {
	/^tmp\/img\/([^.]+)\.xcf-(\d+)-([a-zA-Z0-9_\-]+)(?:@([a-zA-Z0-9_\-]+)(?:#([a-zA-Z0-9_\-]+))?)?((?:\s+-?\d+(?:\.\d+)?_-?\d+(?:\.\d+)?)*)\.png$/s
		or die "Unparsable filename: $_\n";
	my ($xcf, $num, $imgset, $pose, $variant, $data) = ($1, $2, $3, $4, $5, $6);
	my @data = map {/^(.*)_(.*)$/; [$1, $2]} (split /\s+/, $data);
	shift @data;
	if (defined $pose) {
		if (defined $variant) {
			$images{$xcf}{$imgset}{$pose}{$variant} = { _NUM => $num, _FILE => $_ };
		}
		else {
			$images{$xcf}{$imgset}{$pose}{_NUM} = $num;
			$images{$xcf}{$imgset}{$pose}{_DATA} = [@data];
		}
	}
	else {
		$images{$xcf}{$imgset}{_NUM} = $num;
		0+@data or die "$xcf: layer $imgset must have its size in its name.\n";
		$images{$xcf}{$imgset}{_DATA} = [@data];
		$images{$xcf}{$imgset}{_SIZE} = shift @{$images{$xcf}{$imgset}{_DATA}};
	}
}
our $num_images = 0;
for my $xcf (keys %images) {
	$num_images += 0+(grep { $_ !~ /^_/ } keys %{$images{$xcf}});
}

use Data::Dumper;
#print Dumper(\%images);

sub sortify {
	my ($h) = @_;
	return
		sort { $h->{$b}{_NUM} <=> $h->{$a}{_NUM} }
		grep { $_ !~ /^_/ }
		keys %$h;
}

 # Sorting, validation, more processing
for my $xcfk (keys %images) {
	die if $xcfk =~ /^_/;
	my $xcf = $images{$xcfk};
	my @imgsets = sortify($xcf);
	$xcf->{_SORTED} = [@imgsets];
	for my $imgsetk (@imgsets) {
		die if $imgsetk =~ /^_/;
		my $imgset = $xcf->{$imgsetk};
		my @poses = sortify($imgset);
		$imgset->{_SORTED} = [@poses];
		my @fvs;
		for my $posek (@poses) {
			die if $posek =~ /^_/;
			my $pose = $imgset->{$posek};
			my @vs = sortify($pose);
			$pose->{_SORTED} = [@vs];
			if (@fvs) {
				if (@vs != @fvs or grep { $vs[$_] ne $fvs[$_] } (0..$#vs)) {
					warn "$0: Warning: Mismatching % variants in $xcf.xcf-$imgsetk\@{$poses[0],$posek}\n";
				}
			}
			else {
				@fvs = @vs;
			}
			$pose->{_DATA} = [@{$imgset->{_DATA}}, @{$pose->{_DATA}}];
		}
		$imgset->{_FILE} = "img/$xcfk/$imgsetk.png";
		$imgset->{_N_POSES} = 0+@poses;
		$imgset->{_N_VARS} = 0+@fvs;
		$imgset->{_VARS} = [@fvs];
	}
}

 # build images
for my $xcfk (sort keys %images) {
	my $xcf = $images{$xcfk};
	for my $imgsetk (@{$xcf->{_SORTED}}) {
		my $imgset = $xcf->{$imgsetk};
		mkdir "built/img/$xcfk";
		print "building $imgset->{_FILE}\n";
		my @infiles = map {
			my $posek = $_;
			map {
				$imgset->{$posek}{$_}{_FILE};
			} @{$imgset->{$posek}{_SORTED}}
		} @{$imgset->{_SORTED}};
		#print "@infiles\n";
		system 'montage', @infiles,
			'-tile', "$imgset->{_N_VARS}x",
			'-geometry', '100%+0+0',
			'-background', 'none',
			"png32:built/$imgset->{_FILE}";
	}
}


 # This will be used in imgs.cpp.epl

1;


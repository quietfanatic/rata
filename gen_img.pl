#!/usr/bin/perl
use strict;
use warnings;

my @imgs = map {
	/^img\/([^;]*)(?:\;(?:(\d+)x(\d+),)?(\d+(?:\.\d*)?),(\d+(?:\.\d*)?))?\.png$/ or die "Error: Weird image filename: $_\n";
	my ($id, $w, $h, $x, $y) = ($1, $2, $3, $4, $5);
	$id =~ s/[^a-zA-Z0-9_]/_/g;
	+{
		file => $_,
		id => $id,
		w => $w // 0,
		h => $h // 0,
		x => $x // 0,
		y => $y // 0, 
	}
} grep {
	/\.png$/
} glob 'img/*';



print <<"END";

namespace img {
	struct Image {
		sf::Image sfi;
		uint w;
		uint h;
		float x;
		float y;
		uint numsubs () {
			if (w == 0 && h == 0) return 1;
			else return (sfi.GetWidth() / w) * (sfi.GetHeight() / h);
		}
	}
${\(join ",\n", map "\t$_->{id}", @imgs)};

	img::Image* _bgs [] = {
${\(join ",\n", map "\t\t&$_->{id}", grep {$_->{id} =~ /bg(?:\d+)/} @imgs)}
	};

	void load_img () {
		bool good = true;
${\(join "\n", map "\t\tgood &= img::$_->{id}.sfi.LoadFromFile(\"$_->{file}\"); img::$_->{id}.sfi.SetSmooth(0);\n\t\timg::$_->{id}.w = $_->{w}; img::$_->{id}.h = $_->{h}; img::$_->{id}.x = $_->{x}; img::$_->{id}.y = $_->{y};", @imgs)}
		if (!good) fprintf(stderr, "Error: At least one image failed to load!\\n");
	}
}


END


#!/usr/bin/perl
use strict;
use warnings;

my @imgs = map {
	/^img\/(?:\d+-)?([^;]*)(?:\;(?:(\d+)x(\d+),)?(\d+(?:\.\d*)?),(\d+(?:\.\d*)?))?\.png$/ or die "Error: Weird image filename: $_\n";
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
		const char* file;
		uint w;
		uint h;
		float x;
		float y;
		uint numsubs () {
			if (w == 0 && h == 0) return 1;
			else return (sfi.GetWidth() / w) * (sfi.GetHeight() / h);
		}
	};

	enum {
${\(join "\n", map "\t\t$_->{id},", @imgs)}
		n_imgs
	};
	img::Image def [] = {
${\(join ",\n", map "\t\t{sf::Image(), \"$_->{file}\", $_->{w}, $_->{h}, $_->{x}, $_->{y}}", @imgs)}
	};

}
void load_img () {
	for (uint i=0; i < img::n_imgs; i++) {
		if (!img::def[i].sfi.LoadFromFile(img::def[i].file))
			printf("Error: Failed to load image %s.\\n", img::def[i].file);
		img::def[i].sfi.SetSmooth(0);
	}
}


END


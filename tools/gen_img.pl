#!/usr/bin/perl
use strict;
use warnings;

my @imgs = map {
	/^img\/(?:\d+-)?([^;]*)(?:\;(?:(\d+)x(\d+),)?(\d+(?:\.\d*)?),(\d+(?:\.\d*)?))?\.png$/ or die "Error: Weird image filename: $_\n";
	my ($id, $w, $h, $x, $y) = ($1, $2, $3, $4, $5);
	$id =~ s/[^a-zA-Z0-9_]/_/g;
	(my $shf = $_) =~ s/'/'\\''/g;
	unless (`file '$_'` =~ /PNG image data, (\d+) x (\d+),/) { die "Image $_ is not PNG.\n" }
	+{
		file => $_,
		id => $id,
		tw => $1,
		th => $2,
		w => $w // $1,
		h => $h // $2,
		x => $x // 0,
		y => $y // 0,
	}
} grep {
	/\.png$/
} glob 'img/*';



print <<"END";

namespace img {
	struct Def {
		GLuint tex;
		const char* file;
		uint tw;
		uint th;
		uint w;
		uint h;
		float x;
		float y;
		uint numsubs () {
			return (tw / w) * (th / h);
		}
	};

	Def def [] = {
${\(join ",\n", map "\t\t{0, \"$_->{file}\", $_->{tw}, $_->{th}, $_->{w}, $_->{h}, $_->{x}, $_->{y}}", @imgs)}
	};
${\(join "\n", map "\timg::Def*const $imgs[$_]{id} = def+$_;", 0..$#imgs)}
	const uint n_imgs = ${\(0+@imgs)};

}
void load_img () {
	for (uint i=0; i < img::n_imgs; i++) {
		img::def[i].tex = SOIL_load_OGL_texture(img::def[i].file, 4, 0, 0);
		if (!img::def[i].tex)
			printf("Error: Failed to load image %s.\\n", img::def[i].file);
		glBindTexture(GL_TEXTURE_2D, img::def[i].tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
}


END


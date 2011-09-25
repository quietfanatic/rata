#!/usr/bin/perl


print <<'END';

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
END

my @imgs = grep /\.png$/ && ! /^img\/\!/, glob 'img/*';
for (@imgs) {
	$_ =~ /^img\/([^;]*)(?:\;(?:\d+x\d+,)?(\d+(?:\.\d*)?),(\d+(?:\.\d*)?))?\.png$/ or die "Error: Weird image filename: $_\n";
	my $id = $1;
	$id =~ s/[^a-zA-Z0-9_]/_/g;
	print "\t$id,\n";
}

print "\t_COMMA_EATER;\n\n\timg::Image* _bgs [] = {\n";

my @bgs;
for (@imgs) {
	if (/img\/bg(\d+)-/) {
		@bgs[$1] = $_;
	}
}
for (@bgs) {
	$_ =~ /^img\/([^;]*)(?:\;(?:\d+x\d+,)?(\d+(?:\.\d*)?),(\d+(?:\.\d*)?))?\.png$/ or die "Error: Weird image filename: $_\n";
	my $id = $1;
	$id =~ s/[^a-zA-Z0-9_]/_/g;
	print "\t\t\&$id,\n";
}

print "\t\tNULL\n\t};\n}\n\nvoid load_img () {\n\tbool good = true;\n";
for (@imgs) {
	$_ =~ /^img\/([^;]*)(?:\;(?:(\d+)x(\d+),)?(\d+(?:\.\d*)?),(\d+(?:\.\d*)?))?\.png$/ or die "Error: Weird image filename: $_\n";
	my ($id, $w, $h, $x, $y) = ($1, $2, $3, $4, $5);
	$id =~ s/[^a-zA-Z0-9_]/_/g;
	$w //= 0; $h //= 0; $x //= 32; $y //= 48;  # center of Rata images
	print "\tgood &= img::$id.sfi.LoadFromFile(\"$_\"); img::$id.sfi.SetSmooth(0);\n\timg::$id.w = $w; img::$id.h = $h;\n\timg::$id.x = $x; img::$id.y = $y;\n";
}


print "\tif (!good) fprintf(stderr, \"Error: At least one image failed to load!\\n\");\n}\n\n\n\n";


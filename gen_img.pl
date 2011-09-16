#!/usr/bin/perl


print <<'END';

namespace img {
	struct Image {
		sf::Image sfi;
		float x;
		float y;
	}
END

my @imgs = grep /\.png$/ && ! /^img\/\!/, glob 'img/*';
for (@imgs) {
	$_ =~ /^img\/([^;]*)(?:\;(\d+(?:\.\d*)?),(\d+(?:\.\d*)?))?\.png$/ or die "Error: Weird image filename: $_\n";
	my $id = $1;
	$id =~ s/[^a-zA-Z0-9_]/_/g;
	print "\t$id,\n";
}

print "\t_COMMA_EATER;\n}\n\nvoid load_img () {\n\tbool good = true;\n";
for (@imgs) {
	$_ =~ /^img\/([^;]*)(?:\;(\d+(?:\.\d*)?),(\d+(?:\.\d*)?))?\.png$/ or die "Error: Weird image filename: $_\n";
	my ($id, $x, $y) = ($1, $2, $3);
	$id =~ s/[^a-zA-Z0-9_]/_/g;
	$x //= 32; $y //= 48;  # center of Rata images
	print "\tgood &= img::$id.sfi.LoadFromFile(\"$_\"); img::$id.sfi.SetSmooth(0);\n\timg::$id.x = $x; img::$id.y = $y;\n";
}


print "\tif (!good) fprintf(stderr, \"Error: At least one image failed to load!\\n\");\n}\n\n\n\n";





print "\n\nenum id {\n";

my @rooms = grep /\.room\.c\+\+/, glob 'rooms/*';
for (@rooms) {
	$_ =~ /rooms\/(.*?)\.room\.c\+\+/ or die "Error: Weird room filename: $_\n";
	my $id = $1;
	$id =~ s/[^a-zA-Z0-9_]/_/g;
	print "\t$id,\n";
}

print "};\n\n";

for (@rooms) {
	print "#include \"$_\"\n";
}

print "\n\nRoom list [] = {\n";

for (@rooms) {
	$_ =~ /rooms\/(.*?)\.room\.c\+\+/ or die "Error: Weird room filename: $_\n";
	my $id = $1;
	$id =~ s/[^a-zA-Z0-9_]/_/g;
	print "\tr_$id,\n";
}

print "};\n\n";










print "\n\nnamespace file {\n";

my @rooms = grep /\.room\.c\+\+/, glob 'rooms/*';
for (@rooms) {
	$_ =~ /rooms\/(.*?)\.room\.c\+\+/ or die "Error: Weird room filename: $_\n";
	my $id = $1;
	$id =~ s/[^a-zA-Z0-9_]/_/g;
	print "\tnamespace $id {\n\t\t#include \"$_\"\n\t}\n";
}

print "};\n\n";










print "\n\nnamespace file {\n";

my @rooms = grep /\.room\.c\+\+/, glob 'rooms/*';
for (@rooms) {
	$_ =~ /rooms\/\d+-(.*?)\.room\.c\+\+/ or die "Error: Weird room filename: $_\n";
	my $id = $1;
	$id =~ s/[^a-zA-Z0-9_]/_/g;
	print "\tnamespace $id { extern Room room; }\n";
}
for (@rooms) {
	$_ =~ /rooms\/\d+-(.*?)\.room\.c\+\+/ or die "Error: Weird room filename: $_\n";
	my $id = $1;
	$id =~ s/[^a-zA-Z0-9_]/_/g;
	print "\tnamespace $id {\n\t\t#include \"$_\"\n\t}\n";
}
my $count = 0+@rooms;
print <<"END";
};

uint n_rooms = $count;
Room* list [] = {
END

for (@rooms) {
	$_ =~ /rooms\/\d+-(.*?)\.room\.c\+\+/ or die "Error: Weird room filename: $_\n";
	my $id = $1;
	$id =~ s/[^a-zA-Z0-9_]/_/g;
	print "\t&file::${id}::room,\n";
}

print <<'END';
};
END






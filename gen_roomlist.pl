
my @rooms = grep /\.room\.c\+\+/, glob 'rooms/*';

print <<'END';


enum ID {
END

for (@rooms) {
	$_ =~ /rooms\/\d+-(.*?)\.room\.c\+\+/ or die "Error: Weird room filename: $_\n";
	my $id = $1;
	$id =~ s/[^a-zA-Z0-9_]/_/g;
	print "\t$id,\n";
}

print <<'END';
	n_rooms
};

namespace file {
END

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
print <<'END';
};

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









print "\n\nnamespace file {\n";

my @rooms = grep /\.room\.c\+\+/, glob 'rooms/*';
for (@rooms) {
	$_ =~ /rooms\/(.*?)\.room\.c\+\+/ or die "Error: Weird room filename: $_\n";
	my $id = $1;
	$id =~ s/[^a-zA-Z0-9_]/_/g;
	print "\tnamespace $id {\n\t\t#include \"$_\"\n\t}\n";
}

print <<'END';
};
#ifdef MAPEDITOR

struct room_pair {
	char* k;
	Room* v;
	Room* get (char* k_) {
		if (strcmp(k, k_) == 0) return v;
		else return NULL;
	}
} list [] = {
END

for (@rooms) {
	$_ =~ /rooms\/(.*?)\.room\.c\+\+/ or die "Error: Weird room filename: $_\n";
	my $id = my $name = $1;
	$id =~ s/[^a-zA-Z0-9_]/_/g;
	print "\t{\"$name\", &file::${id}::room},\n";
}

print <<'END';
	{NULL, NULL}
};
Room* name (char* k) {
	for (room_pair* p = list; p->k; p++) {
		if (Room* r = p->get(k)) return r;
	}
	return NULL;
}
#endif

END








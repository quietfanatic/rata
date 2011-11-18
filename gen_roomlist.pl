#!/usr/bin/perl
use strict;
use warnings;


my @rooms = map {
	$_ =~ /rooms\/\d+-(.*?)\.room\.c\+\+/ or die "Error: Weird room filename: $_\n";
	my $id = $1;
	$id =~ s/[^a-zA-Z0-9_]/_/g;
	{
		file => $_,
		id => $id,
	}
} grep {
	/\.room\.c\+\+/
} glob 'rooms/*';

print <<"END";


enum ID {
${\(join ",\n", map "\t$_->{id}", @rooms)},
	n_rooms
};

namespace file {
${\(join "\n", map "\tnamespace $_->{id} { extern Room room; }", @rooms)}
${\(join "\n", map "\tnamespace $_->{id} {\n\t\t#define THIS_ROOM room::$_->{id}\n\t\t#include \"$_->{file}\"\n\t}", @rooms)}
}

Room* list [] = {
${\(join ",\n", map "\t&file::$_->{id}::room", @rooms)}
};

#ifdef MAPEDITOR
char* filename [] = {
${\(join ",\n", map "\t\"$_->{file}\"", @rooms)}
};
#endif

END




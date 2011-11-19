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
${\(join "\n", map "\tnamespace $_->{id} {\n\t\t#define THIS_ROOM room::$_->{id}\n\t\t#include \"$_->{file}\"\n\t\t#undef THIS_ROOM\n\t}", @rooms)}
}

Def def [n_rooms] = {
${\(join ",\n", map "\tfile::$_->{id}::def", @rooms)}
};


END




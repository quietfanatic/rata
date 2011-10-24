#!/usr/bin/perl
use strict;
use warnings;
my @snds = map {
	$_ =~ /^snd\/(.*?)\.(?:flac|ogg)$/ or die "Error: Weird sound filename: $_\n";
	my $id = $1;
	$id =~ s/[^a-zA-Z0-9_]/_/g;
	{
		file => $_,
		id => $id,
	}
} grep {
	/\.(?:flac|ogg)$/
} glob 'snd/*';
my @bgms = map {
	$_ =~ /^bgm\/\d+-(.*?)\.(?:flac|ogg)$/ or die "Error: Weird sound filename: $_\n";
	my $id = $1;
	$id =~ s/[^a-zA-Z0-9_]/_/g;
	{
		file => $_,
		id => $id,
	}
} grep {
	/\.(?:flac|ogg)$/
} glob 'bgm/*';

print <<"END";

namespace snd {
	struct Sound {
		sf::SoundBuffer sfsb;
		sf::Sound sfs;
		const char* file;
		void stop () {
			sfs.Stop();
		}
		void play (float pitch=1.0, float volume=100.0) {
			sfs.SetVolume(volume);
			sfs.SetPitch(pitch);
			sfs.Play();
		}
	};
	enum {
${\(join "\n", map "\t\t$_->{id},", @snds)}
		n_snds
	};
	Sound def [n_snds] = {
${\(join ",\n", map "\t\t{sf::SoundBuffer(), sf::Sound(), \"$_->{file}\"}", @snds)}
	};
}

namespace bgm {
	enum {
${\(join "\n", map "\t\t$_->{id},", @bgms)}
		n_bgms
	};

	char* name [n_bgms] = {
${\(join ",\n", map "\t\t\"$_->{file}\"", @bgms)}
	};
	sf::Music music;
}

void load_snd () {
	for (uint i=0; i < snd::n_snds; i++) {
		if (!snd::def[i].sfsb.LoadFromFile(snd::def[i].file))
			printf("Error: Failed to load sound %s.\\n", snd::def[i].file);
		snd::def[i].sfs.SetBuffer(snd::def[i].sfsb);
	}
}


END


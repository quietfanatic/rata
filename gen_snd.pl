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
	$_ =~ /^bgm\/(.*?)\.(?:flac|ogg)$/ or die "Error: Weird sound filename: $_\n";
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
		void stop () {
			sfs.Stop();
		}
		void play (float pitch=1.0, float volume=100.0) {
			sfs.SetVolume(volume);
			sfs.SetPitch(pitch);
			sfs.Play();
		}
	}
${\(join ",\n", map "\t$_->{id}", @snds)};
}

namespace bgm {
	sf::Music
${\(join ",\n", map "\t$_->{id}", @bgms)};
}

void load_snd () {
	bool good = true;
${\(join "\n", map "\tgood &= snd::$_->{id}.sfsb.LoadFromFile(\"$_->{file}\"); \n\tsnd::$_->{id}.sfs.SetBuffer(snd::$_->{id}.sfsb);", @snds)}
${\(join "\n", map "\tgood &= bgm::$_->{id}.OpenFromFile(\"$_->{file}\"); bgm::$_->{id}.SetLoop(true);", @bgms)}
	if (!good) fprintf(stderr, "Error: At least one sound or bgm failed to load!\\n");
}


END


#!/usr/bin/perl


print <<'END';

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
END

my @snds = grep /\.(?:flac|ogg)$/ && ! /^snd\/\!/, glob 'snd/*';
for (@snds) {
	$_ =~ /^snd\/(.*?)\.(?:flac|ogg)$/ or die "Error: Weird sound filename: $_\n";
	my $id = $1;
	$id =~ s/[^a-zA-Z0-9_]/_/g;
	print "\t$id,\n";
}

print "\t_COMMA_EATER;\n\n}\n\nnamespace bgm {\n\tsf::Music\n";
my @bgms = grep /\.(?:flac|ogg)$/ && ! /^bgm\/\!/, glob 'bgm/*';
for (@bgms) {
	$_ =~ /^bgm\/(.*?)\.(?:flac|ogg)$/ or die "Error: Weird music filename: $_\n";
	my $id = $1;
	$id =~ s/[^a-zA-Z0-9_]/_/g;
	print "\t$id,\n";
}
print "\t_COMMA_EATER;\n\n}\n\nvoid load_snd () {\n\tbool good = true;\n";
for (@snds) {
	$_ =~ /^snd\/(.*?).(?:flac|ogg)$/ or die "Error: Weird sound filename: $_\n";
	my $id = $1;
	$id =~ s/[^a-zA-Z0-9_]/_/g;
	print "\tgood &= snd::$id.sfsb.LoadFromFile(\"$_\");\n\tsnd::$id.sfs.SetBuffer(snd::$id.sfsb);\n";
}
for (@bgms) {
	$_ =~ /^bgm\/(.*?)\.(?:flac|ogg)$/ or die "Error: Weird music filename: $_\n";
	my $id = $1;
	$id =~ s/[^a-zA-Z0-9_]/_/g;
	print "\tgood &= bgm::$id.OpenFromFile(\"$_\"); bgm::$id.SetLoop(true);\n";
}


print "\tif (!good) fprintf(stderr, \"Error: At least one sound failed to load!\\n\");\n}\n\n\n\n";



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
		fall,
		gunshot,
		hit,
		hurt,
		ricochet,
		squeak,
		step,
		woodhit,
		n_snds
	};
	Sound def [n_snds] = {
		{sf::SoundBuffer(), sf::Sound(), "snd/fall.ogg"},
		{sf::SoundBuffer(), sf::Sound(), "snd/gunshot.flac"},
		{sf::SoundBuffer(), sf::Sound(), "snd/hit.ogg"},
		{sf::SoundBuffer(), sf::Sound(), "snd/hurt.ogg"},
		{sf::SoundBuffer(), sf::Sound(), "snd/ricochet.ogg"},
		{sf::SoundBuffer(), sf::Sound(), "snd/squeak.ogg"},
		{sf::SoundBuffer(), sf::Sound(), "snd/step.ogg"},
		{sf::SoundBuffer(), sf::Sound(), "snd/woodhit.ogg"}
	};
}

namespace bgm {
	enum {
		theme,
		life,
		safe,
		n_bgms
	};

	char* name [n_bgms] = {
		"bgm/0-theme.ogg",
		"bgm/1-life.ogg",
		"bgm/2-safe.ogg"
	};
	sf::Music music;
}

void load_snd () {
	for (uint i=0; i < snd::n_snds; i++) {
		if (!snd::def[i].sfsb.LoadFromFile(snd::def[i].file))
			printf("Error: Failed to load sound %s.\n", snd::def[i].file);
		snd::def[i].sfs.SetBuffer(snd::def[i].sfsb);
	}
}



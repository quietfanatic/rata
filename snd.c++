
namespace snd {
	struct Sound {
		sf::SoundBuffer sfsb;
		sf::Sound sfs;
		void play () {
			sfs.Play();
		}
		void stop () {
			sfs.Stop();
		}
		void play (float pitch) {
			sfs.SetPitch(pitch);
			sfs.Play();
		}
	}
	gunshot,
	hurt,
	squeak,
	_COMMA_EATER;

}

void load_snd () {
	bool good = true;
	good &= snd::gunshot.sfsb.LoadFromFile("snd/gunshot.flac");
	snd::gunshot.sfs.SetBuffer(snd::gunshot.sfsb);
	good &= snd::hurt.sfsb.LoadFromFile("snd/hurt.ogg");
	snd::hurt.sfs.SetBuffer(snd::hurt.sfsb);
	good &= snd::squeak.sfsb.LoadFromFile("snd/squeak.ogg");
	snd::squeak.sfs.SetBuffer(snd::squeak.sfsb);
	if (!good) fprintf(stderr, "Error: At least one sound failed to load!\n");
}





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
	_COMMA_EATER;

}

void load_snd () {
	bool good = true;
	good &= snd::gunshot.sfsb.LoadFromFile("snd/gunshot.flac");
	snd::gunshot.sfs.SetBuffer(snd::gunshot.sfsb);
	if (!good) fprintf(stderr, "Error: At least one sound failed to load!\n");
}




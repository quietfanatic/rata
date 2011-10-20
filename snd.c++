
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
	gunshot,
	hurt,
	ricochet,
	squeak,
	step,
	_COMMA_EATER;

}

void load_snd () {
	bool good = true;
	good &= snd::gunshot.sfsb.LoadFromFile("snd/gunshot.flac");
	snd::gunshot.sfs.SetBuffer(snd::gunshot.sfsb);
	good &= snd::hurt.sfsb.LoadFromFile("snd/hurt.ogg");
	snd::hurt.sfs.SetBuffer(snd::hurt.sfsb);
	good &= snd::ricochet.sfsb.LoadFromFile("snd/ricochet.ogg");
	snd::ricochet.sfs.SetBuffer(snd::ricochet.sfsb);
	good &= snd::squeak.sfsb.LoadFromFile("snd/squeak.ogg");
	snd::squeak.sfs.SetBuffer(snd::squeak.sfsb);
	good &= snd::step.sfsb.LoadFromFile("snd/step.ogg");
	snd::step.sfs.SetBuffer(snd::step.sfsb);
	if (!good) fprintf(stderr, "Error: At least one sound failed to load!\n");
}




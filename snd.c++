
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
	fall,
	gunshot,
	hit,
	hurt,
	ricochet,
	squeak,
	step,
	woodhit,
	_COMMA_EATER;

}

namespace bgm {
	sf::Music
	life,
	_COMMA_EATER;

}

void load_snd () {
	bool good = true;
	good &= snd::fall.sfsb.LoadFromFile("snd/fall.ogg");
	snd::fall.sfs.SetBuffer(snd::fall.sfsb);
	good &= snd::gunshot.sfsb.LoadFromFile("snd/gunshot.flac");
	snd::gunshot.sfs.SetBuffer(snd::gunshot.sfsb);
	good &= snd::hit.sfsb.LoadFromFile("snd/hit.ogg");
	snd::hit.sfs.SetBuffer(snd::hit.sfsb);
	good &= snd::hurt.sfsb.LoadFromFile("snd/hurt.ogg");
	snd::hurt.sfs.SetBuffer(snd::hurt.sfsb);
	good &= snd::ricochet.sfsb.LoadFromFile("snd/ricochet.ogg");
	snd::ricochet.sfs.SetBuffer(snd::ricochet.sfsb);
	good &= snd::squeak.sfsb.LoadFromFile("snd/squeak.ogg");
	snd::squeak.sfs.SetBuffer(snd::squeak.sfsb);
	good &= snd::step.sfsb.LoadFromFile("snd/step.ogg");
	snd::step.sfs.SetBuffer(snd::step.sfsb);
	good &= snd::woodhit.sfsb.LoadFromFile("snd/woodhit.ogg");
	snd::woodhit.sfs.SetBuffer(snd::woodhit.sfsb);
	good &= bgm::life.OpenFromFile("bgm/life.ogg"); bgm::life.SetLoop(true);
	if (!good) fprintf(stderr, "Error: At least one sound failed to load!\n");
}




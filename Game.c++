

#ifdef HEADER

struct Game;

#else

struct Game : Actor {
	Game (actor::Def* def) : Actor(def) { }
};

#endif



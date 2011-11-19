

#ifdef HEADER

struct Item : Actor {
	Item (actor::Def* def);
	void after_move ();
	void draw ();
};

#else

Item::Item (actor::Def* def) : Actor(def) { }
void Item::after_move () {
	if (rata->floor)
		rata->propose_action(Rata::action_equip, this, pos, 1);
}
void Item::draw () {
	if (data)
		draw_image(
			item::def[data].appearance,
			pos,
			item::def[data].world_frame
		);
}





#endif









#ifdef HEADER

struct Item : Actor {
	void draw ();
	void after_move ();
};

#else

void Item::draw () {
	if (data)
		draw_image(
			item::def[data].appearance,
			pos,
			item::def[data].world_frame
		);
}
void Item::after_move () {
	if (rata->floor)
		rata->propose_action(Rata::action_equip, this, pos, 1);
}





#endif







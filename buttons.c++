




#ifdef HEADER

struct Button {
	Vec pos;
	Vec size;
	void (* click )();
	void (* drag )();
	img::Def* image;
	int16 subimage;
	Color color;
	char* text;

	void draw () const {
		if (color.visible()) draw_rect(
			pos.x, pos.y,
			pos.x+size.x, pos.y+size.y,
			color
		);
		if (image) draw_image(
			image, pos, subimage, false
		);
		if (text) render_text(
			text,
			pos + size.scalex(0.5),
			10, false, false, 0
		);
	}
};

#else

void click_quit () {
	if (button[sf::Mouse::Left] == 1) {
		quit_game();
	}
}



const uint n_pause_buttons = 2;
Button pause_buttons [n_pause_buttons] = {
	{Vec(0, 0), Vec(3, 1), NULL, NULL, NULL, 0, 0, "Options"},
	{Vec(18, 0), Vec(2, 1), &click_quit, NULL, NULL, 0, 0, "Quit"},
};

#endif




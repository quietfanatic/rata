






struct Button {
	Vec pos;
	Vec size;
	void (* click )();
	void (* drag )();
	int16 image;
	int16 subimage;
	Color color;
	char* text;

	inline void draw () const {
		if (color.visible()) draw_rect(
			pos.x, pos.y,
			pos.x+size.x, pos.y+size.y,
			color, true
		);
		if (image) draw_image(
			image, pos, subimage, false, true
		);
		if (text) render_text(
			text,
			Vec(pos.x, pos.y + size.y),
			10, false, false, 1, true
		);
	}
};





const uint n_pause_buttons = 2;
Button pause_buttons [n_pause_buttons] = {
	{Vec(2, 2), Vec(4, 1), NULL, NULL, -1, 0, 0x0000ff7f, NULL},
	{Vec(14, 2), Vec(4, 1), NULL, NULL, -1, 0, 0xff00007f, NULL},
};






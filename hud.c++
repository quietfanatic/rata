




#ifdef HEADER

struct Button {
	Vec pos;
	Vec size;
	void (* click )();
	void (* drag )();
};

#else

void click_quit () {
	if (button[sf::Mouse::Left] == 1) {
		quit_game();
	}
}


void hud_play () {
	 // Message
	if (message_pos)
		render_text(message_pos, vec(10, 1), 1, false, true, 0);
	 // Lifebar
	if (rata) {
		int life = rata->life;
		for (int i = 0; i < (rata->max_life+47)/48; i++) {
			uint subimage;
			if (life >= i*48 + 48)
				subimage = 0;
			else if (life >= i*48 + 36)
				subimage = 1;
			else if (life >= i*48 + 24)
				subimage = 2;
			else if (life >= i*48 + 12)
				subimage = 3;
			else
				subimage = 4;
			draw_image(
				img::heart,
				vec(19.5 - (i * 12*PX), 14.25),
				subimage,
				false
			);
		}
	}
}

void hud_pause () {
	hud_play();
	draw_rect(0, 0, 20, 15, 0x0000004f);
	render_text("Options", vec(0, 1));
	render_text("Quit", vec(20, 1), 1, false, false, -1);
}

const uint n_pause_buttons = 2;
Button pause_buttons [n_pause_buttons] = {
	{{0, 0}, {3, 1}, NULL, NULL},
	{{18, 0}, {2, 1}, &click_quit, NULL},
};












#endif




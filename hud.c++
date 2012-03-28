




#ifdef HEADER

struct Button {
	Vec pos;
	Vec size;
	void (* click )();  // Executed immediately on click
	void (* drag )();  // Executed each frame when dragging.
};
const uint n_pause_buttons = 2;
extern Button pause_buttons [n_pause_buttons];

#else

void click_options () {
	if (button[GLFW_MOUSE_BUTTON_LEFT] == 1) {
		dragging = &pause_buttons[0];
		drag_start = cursor2;
	}
}
void drag_options () {
	if (button[GLFW_MOUSE_BUTTON_LEFT]) {
		pause_buttons[0].pos += cursor2 - drag_start;
		drag_start = cursor2;
	}
	else {
		dragging = NULL;
	}
}

void click_quit () {
	if (button[GLFW_MOUSE_BUTTON_LEFT] == 1) {
		quit_game();
	}
}


void hud_play () {
	 // Message
	if (message_pos)
		render_text(message_pos, Vec(10, 1), 1, false, true, 0);
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
				Vec(19.5 - (i * 12*PX), 14.25),
				subimage,
				false
			);
		}
	}
}

void hud_pause () {
	hud_play();
	draw_rect(Rect(0, 0, 20, 15), 0x0000004f);
	render_text("Options", pause_buttons[0].pos + Vec(0, 1));
	render_text("Quit", Vec(20, 1), 1, false, false, -1);
}

//const uint n_pause_buttons = 2;
Button pause_buttons [n_pause_buttons] = {
	{Vec(0, 0), Vec(3, 1), &click_options, &drag_options},
	{Vec(18, 0), Vec(2, 1), &click_quit, NULL},
};












#endif




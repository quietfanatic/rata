
#ifdef HEADER

void quit_game ();
void main_init ();
void main_loop ();

#else

void quit_game () {
	//window->SetCursorPosition(cursor2.x*UNPX*window_scale, window->GetHeight() - cursor2.y*UNPX*window_scale);
	glfwTerminate();
	throw 0;
}

void set_video () {
	glfwOpenWindow(
		320*window_scale, 240*window_scale,
		8, 8, 8, 8, 0, 0, GLFW_WINDOW
	);
	glfwSwapInterval(1);
	start_trap();
	init_shaders();
	init_graphics();
}

void start_trap () {
	trap_cursor = true;
	glfwDisable(GLFW_MOUSE_CURSOR);
	glfwGetMousePos(&oldmousex, &oldmousey);
}

void stop_trap () {
	trap_cursor = false;
	glfwEnable(GLFW_MOUSE_CURSOR);
}


void main_init () {
	if (!glfwInit()) {
		printf("Error: could not initialize glfw.\n");
		exit(1);
	}
	set_video();
	glfwDisable(GLFW_AUTO_POLL_EVENTS);
	glfwSetWindowCloseCallback(close_cb);
	glfwSetKeyCallback(key_cb);
	glfwSetMouseButtonCallback(btn_cb);
	glfwSetMousePosCallback(mouse_cb);
	start_trap();
	 // box2d
	world = new b2World(
		b2Vec2(0.0, gravity),  // grav
		true  // allow sleep
	);
	world->SetContactListener(new myCL);
	 // Initial room
	//room::testroom.start();
	cursor2 = screen/2;
}

void toggle_pause () {
	if (paused) {
		paused = false;
		start_trap();
		n_buttons = 0;
		draw_hud = &hud_play;
	}
	else {
		paused = true;
		stop_trap();
		n_buttons = n_pause_buttons;
		buttons = pause_buttons;
		draw_hud = &hud_pause;
	}
};



void add_phase () {
	while (activation_queue) {
		Actor* c = activation_queue;
		activation_queue = c->next_active;
		Actor** a;
		for (a = &active_actors; *a; a = &(*a)->next_active) {
			if (type::def[c->type].depth > type::def[(*a)->type].depth) {
				dbg_actor("Linking %08x.\n", c);
				c->next_active = *a;
				*a = c;
				goto done_activating;
			}
		}  // Least deep object
		dbg_actor("Linking %08x.\n", c);
		c->next_active = NULL;
		*a = c;
		done_activating:
		c->awaiting_activation = false;
		c->active = true;
	}
//	printf("-------\n");
//	for (Actor* a = active_actors; a; a = a->next_active) {
//		printf("%08x\n", a);
//	}
}

void remove_phase () {
	for (Actor** a = &active_actors; *a;) {
		if (!(*a)->active) {
			dbg_actor("Unlinking %08x.\n", *a);
			Actor* c = *a;
			*a = c->next_active;
			c->next_active = NULL;
		}
		else { a = &(*a)->next_active; }
	}
//	printf("-------\n");
//	for (Actor* a = active_actors; a; a = a->next_active) {
//		printf("%08x\n", a);
//	}
}


void camera_phase () {
	get_focus();
	get_camera();
}

void draw_phase () {
	draw_latency -= 1/FPS;
	if (draw_latency > 1/FPS) {
		dbg_timing("Skipping frame.\n");
		return;
	}
	
	glClear(GL_COLOR_BUFFER_BIT);
	camera_to_screen();

	 // Draw actors
	for (Actor* a = active_actors; a; a = a->next_active) {
		dbg_draw("Drawing 0x%08x\n", a);

		a->draw();
	}

	debug_draw();

	finish_drawing();

	draw_latency += glfwGetTime();
	glfwSetTime(0);
	if (draw_latency < 0) glfwSleep(-draw_latency);
	dbg_timing("%f\n", draw_latency);
	glfwSwapBuffers();	
}

int GLFWCALL close_cb () {
	quit_game();
	return true;
}

void GLFWCALL key_cb (int keycode, int action) {
	if (action == GLFW_PRESS) {
		switch (keycode) {
			case GLFW_KEY_ESC: {
				quit_game();
			}
			case '=': {
				debug_mode = !debug_mode;
				return;
			}
			case 'P': {
				toggle_pause();
				return;
			}
			default: {
				if (keycode < 400)
					key[keycode] = 1;
				return;
			}
		}
	}
	else {  // action == GLFW_RELEASE
		if (keycode < 400)
			key[keycode] = 0;
		return;
	}
}

void GLFWCALL btn_cb (int btn, int action) {
	if (action == GLFW_PRESS) {
		if (btn < 10) button[btn] = 1;
	}
	else {
		if (btn < 10) button[btn] = 0;
	}
}

void GLFWCALL mouse_cb (int x, int y) {
	if (trap_cursor) {
		cursor += Vec(
			x - oldmousex,
			oldmousey - y
		) * PX * cursor_scale / window_scale;
		if (cursor.x > 19) cursor.x = 19;
		else if (cursor.x < -19) cursor.x = -19;
		if (cursor.y > 14) cursor.y = 14;
		else if (cursor.y < -14) cursor.y = -14;
	}
	else {
		cursor2 = Vec(
			x - oldmousex,
			oldmousey - y
		) * PX / window_scale;
	}
	oldmousex = x;
	oldmousey = y;
}


void input_phase () {
	click_taken = false;
	 // Count frames for each key and button
	for (uint sym = 0; sym < 400; sym++)
	if (key[sym] > 0 and key[sym] < 255) key[sym]++;
	for (uint btn = 0; btn < 10; btn++)
	if (button[btn] > 0 and btn[button] < 255) button[btn]++;
	glfwPollEvents();
};

void move_phase () {
	if (n_buttons) {
		if (dragging) {
			(*dragging->drag)();
		}
		else
		for (uint i=0; i < n_buttons; i++)
		if (in_rect(cursor2, Rect(buttons[i].pos, buttons[i].pos+buttons[i].size))) {
			if (buttons[i].click) (*buttons[i].click)();
			break;
		}
	}
	if (paused) return;
	for (Actor* a = active_actors; a; a = a->next_active)
		a->before_move();

	for (uint i=0; i < MAX_BULLETS; i++)
		bullets[i].move();
	if (world) {
		world->SetAutoClearForces(false);
		world->Step(1/120.0, 10, 10);
		world->SetAutoClearForces(true);
		world->Step(1/120.0, 10, 10);
		//world->Step(1/180.0, 10, 10);
		//world->Step(1/240.0, 10, 10);
	}
	 // We need to reset attentions before actors register more.
	reset_attentions();

	for (Actor* a = active_actors; a; a = a->next_active)
		a->after_move();
}


void main_loop () {
	reset_attentions();
	for (;;) {
		frame_number++;
		add_phase();
		camera_phase();
		draw_phase();
		input_phase();
		move_phase();
		remove_phase();
	}
}


#endif


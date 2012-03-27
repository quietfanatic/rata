
#ifdef HEADER

void quit_game ();
void main_init ();
void main_loop ();

#else

void quit_game () {
	window->SetCursorPosition(cursor2.x*UNPX*window_scale, window->GetHeight() - cursor2.y*UNPX*window_scale);
	window->Close();
	throw 0;
}


void main_init () {
	 // SFML
	window = new sf::Window;
	//window = new sf::Image(320, 240);
	set_video();
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
		trap_cursor = true;
		window->ShowMouseCursor(false);
		window->SetCursorPosition(window->GetWidth()/2, window->GetHeight()/2);
		n_buttons = 0;
		draw_hud = &hud_play;
	}
	else {
		paused = true;
		trap_cursor = false;
		window->SetCursorPosition(cursor2.x*UNPX*window_scale, window->GetHeight() - cursor2.y*UNPX*window_scale);
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
	
	camera_to_screen();

	 // Draw actors
	for (Actor* a = active_actors; a; a = a->next_active) {
		dbg_draw("Drawing 0x%08x\n", a);

		a->draw();
	}

	debug_draw();

	finish_drawing();

	window->ShowMouseCursor(!trap_cursor);
	draw_latency += frameclock.GetElapsedTime();
	frameclock.Reset();
	if (draw_latency < 0) sf::Sleep(-draw_latency);
	dbg_timing("%f\n", draw_latency);
	window->Display();
}

void input_phase () {
	click_taken = false;
	 // Count frames for each key and button
	for (uint sym = 0; sym < 400; sym++)
	if (key[sym] > 0 and key[sym] < 255) key[sym]++;
	for (uint btn = 0; btn < 10; btn++)
	if (button[btn] > 0 and btn[button] < 255) button[btn]++;
	 // Start event handling
	sf::Event event;
	while (window->GetEvent(event))
	switch (event.Type) {
		case sf::Event::KeyPressed: {
			if (event.Key.Code == sf::Key::Escape) {
				quit_game();
			}
			if (event.Key.Code == sf::Key::Equal) debug_mode = !debug_mode;
			if (event.Key.Code == sf::Key::F11) {
				window_fullscreen = !window_fullscreen;
				set_video();
			}
			//if (event.Key.Code == sf::Key::Num1) enter_room(room::test1, 0);
			//if (event.Key.Code == sf::Key::Num2) enter_room(room::test2, 0);
			//if (event.Key.Code == sf::Key::Num3) enter_room(room::test3, 0);
			//if (event.Key.Code == sf::Key::Num4) enter_room(room::test4, 0);
			//if (event.Key.Code == sf::Key::Num5) enter_room(room::edit1, 0);
			//if (event.Key.Code == sf::Key::Num6) enter_room(room::empty, 0);
			//if (event.Key.Code == sf::Key::Num0) save_save();
			if (event.Key.Code == sf::Key::P) toggle_pause();
			if (event.Key.Code >= 400) break;
			key[event.Key.Code] = 1;
			break;
		}
		case sf::Event::KeyReleased: {
			if (event.Key.Code >= 400) break;
			key[event.Key.Code] = 0;
			break;
		}
		case sf::Event::MouseButtonPressed: {
			if (event.MouseButton.Button >= 10) break;
			button[event.MouseButton.Button] = 1;
			break;
		}
		case sf::Event::MouseButtonReleased: {
			if (event.MouseButton.Button >= 10) break;
			button[event.MouseButton.Button] = 0;
			break;
		}
		case sf::Event::MouseMoved: {
			if (trap_cursor) {
				cursor += Vec(
					(event.MouseMove.X - window->GetWidth()/2.0),
					-(event.MouseMove.Y - window->GetHeight()/2.0)
				) * PX * cursor_scale / window_scale;
				window->SetCursorPosition(window->GetWidth()/2, window->GetHeight()/2);
				if (cursor.x > 19) cursor.x = 19;
				else if (cursor.x < -19) cursor.x = -19;
				if (cursor.y > 14) cursor.y = 14;
				else if (cursor.y < -14) cursor.y = -14;
			}
			else {
				cursor2 = Vec(
					event.MouseMove.X,
					(window->GetHeight() - event.MouseMove.Y)
				) * PX / window_scale;
			}
			break;
		}
		case sf::Event::Closed: {
			quit_game();
			break;
		}
	}
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
	window->SetCursorPosition(window->GetWidth()/2, window->GetHeight()/2);
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


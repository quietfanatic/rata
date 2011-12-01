
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

void set_video () {
	if (window_fullscreen) {
		window->Create(sf::VideoMode(640, 480, 32), "", sf::Style::Fullscreen);
	}
	else {
		window->Create(sf::VideoMode(320*window_scale, 240*window_scale, 32), "");
	}
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glTranslatef(-1, -1, 0);
	glScalef(1/20.0, 1/15.0, 1);
	glTranslatef(0.45*PX/2, 0.45*PX/2, 0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	window->UseVerticalSync(true);
	//window->SetFramerateLimit(60);
	window->EnableKeyRepeat(false);
	window->ShowMouseCursor(false);
	window->Display();
	frameclock.Reset();
	draw_latency = 0;
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
				printf("Linking %08x.\n", c);
				c->next_active = *a;
				*a = c;
				goto done_activating;
			}
		}  // Least deep object
		printf("Linking %08x.\n", c);
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
			printf("Unlinking %08x.\n", *a);
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
	 // To look smooth in a pixelated environment,
	 //  we need a minimum speed.
	 // We also need to hold camera pixel-steady
	 //  to Rata pos when running.
	float snap_dist = camera_snap ? .5*PX : .25*PX;
	camera_snap = false;
	if (camera_jump) {
		camera = focus;
		camera_jump = false;
	}
	else {
		if (abs_f(focus.x - camera.x) < .25*PX) camera.x = focus.x;
		else {
			float xvel = (focus.x - camera.x) / 10;
			if (abs_f(xvel) < .25*PX)
				camera.x += .25*PX * sign_f(xvel);
			else if (abs_f((xvel) - rata->vel.x/FPS) < snap_dist) {
				camera.x = old_camera_rel + round(rata->pos.x*UNPX)*PX;
				camera_snap = true;
			}
			else
				camera.x += xvel;
		}
		if (abs_f(focus.y - camera.y) < .25*PX) camera.y = focus.y;
		else {
			float yvel = (focus.y - camera.y) / 10;
			if (abs_f(yvel) < .25*PX)
				camera.y += .25*PX * sign_f(yvel);
			else
				camera.y += yvel;
		}
		camera = constrain(camera);
	}
	old_camera_rel = camera.x - round(rata->pos.x*UNPX)*PX;
}

void draw_phase () {
	draw_latency -= 1/FPS;
	if (draw_latency > 1/FPS) {
		dbg(6, "Skipping frame %d.\n", frame_number);
		return;
	}
	 // Move GL view to camera pos
	glTranslatef(
		- round(camera.x*UNPX)*PX + screen.x/2,
		- round(camera.y*UNPX)*PX + screen.y/2,
		0
	);

	 // Draw actors
	for (Actor* a = active_actors; a; a = a->next_active) {
		dbg(8, "Drawing 0x%08x\n", a);

		a->draw();
	}

	 // DEBUG DRAWING
	if (debug_mode)
	for (Actor* a=active_actors; a; a = a->next_active) {
		if (a->has_body()) {
			Object* o = (Object*) a;
			for (b2Fixture* f = o->body->GetFixtureList(); f; f = f->GetNext())
			if (f->GetFilterData().categoryBits)
			switch (f->GetType()) {
				case (b2Shape::e_edge): {
					b2EdgeShape* e = (b2EdgeShape*)f->GetShape();
					draw_line(
						o->pos + Vec(e->m_vertex1),
						o->pos + Vec(e->m_vertex2),
						0x00ff007f
					);
					if (mag2(rata->cursor_pos() - o->pos + Vec(e->m_vertex1)) < 1)
						draw_line(
							o->pos + Vec(e->m_vertex1),
							o->pos + Vec(e->m_vertex0) + Vec(3, 3)*PX,
							0xffff007f
						);
					if (mag2(rata->cursor_pos() - o->pos + Vec(e->m_vertex2)) < 1)
						draw_line(
							o->pos + Vec(e->m_vertex2),
							o->pos + Vec(e->m_vertex3) - Vec(3, 3)*PX,
							0x0000ff7f
						);
					break;
				}
				case (b2Shape::e_polygon): {
					b2PolygonShape* p = (b2PolygonShape*)f->GetShape();					
					Color color = f->GetFilterData().categoryBits == 256 ? 0x0000ff4f : 0x00ff007f;
					glDisable(GL_TEXTURE_2D);
					color.setGL();
					glBegin(GL_LINE_LOOP);
					for (int i=0; i < p->m_vertexCount; i++) {
						vertex(o->pos + Vec(p->m_vertices[i]));
					}
					glEnd();
					break;
				}
				case (b2Shape::e_circle): {
					b2CircleShape* c = (b2CircleShape*)f->GetShape();
					Color color = f->GetFilterData().categoryBits == 256 ? 0x0000ff4f : 0x00ff007f;
					draw_circle(Vec(c->m_p)+o->pos, c->m_radius, color);
					break;
				}
				default: { }
			}
		}
//		else {  // Debug draw an object without a b2Body
//			window->Draw(sf::Shape::Line(
//				a->pos, a->pos + a->vel,
//				1.0*PX, Color(0xff00007f)
//			));
//		};
		 // Debug draw rata path.
		if (mag2(rata->pos - oldratapos) > 0.2) {
			debug_path[debug_path_pos % debug_path_size] = rata->pos;
			uint8 whiteshift = rata->float_frames * 255.0 / (rata->stats.float_time*FPS);
			debug_path_color[debug_path_pos % debug_path_size] =
				whiteshift ? Color(255, whiteshift, whiteshift, 127) : 0x0000ff7f;
			oldratapos = rata->pos;
			debug_path_pos++;
		}

		uint i = debug_path_pos>=debug_path_size ? debug_path_pos-debug_path_size+1 : 0;
		glBegin(GL_LINE_STRIP);
		for (; i < debug_path_pos; i++) {
			debug_path_color[i % debug_path_size].setGL();
			vertex(debug_path[i % debug_path_size]);
		}
		glEnd();
		 // Debug draw Camera walls.
		Color(0x7f007f4f).setGL();
		glBegin(GL_LINES);
		for (uint i=0; i < current_room->n_walls; i++) {
			vertex(current_room->sides[i].a);
			vertex(current_room->sides[i].b);
		}
		glEnd();
		for (uint i=0; i < current_room->n_walls; i++)
		if (current_room->walls[i].r > 0) {
			draw_circle(
				current_room->walls[i].c,
				current_room->walls[i].r,
				0x7f007f4f
			);
		}
		 // Draw camera and focus
		Color(0x007f7f4f).setGL();
		for (uint i=0; i < MAX_ATTENTIONS; i++) {
			glBegin(GL_LINE_LOOP);
				vertex(Vec(attention[i].range.l, attention[i].range.b));
				vertex(Vec(attention[i].range.r, attention[i].range.b));
				vertex(Vec(attention[i].range.r, attention[i].range.t));
				vertex(Vec(attention[i].range.l, attention[i].range.t));
			glEnd();
		}
		draw_rect(Rect(focus - Vec(1, 1)*PX, focus + Vec(1, 1)*PX), 0x0000ff7f);
		draw_rect(Rect(camera - Vec(1, 1)*PX, camera + Vec(1, 1)*PX), 0xff00007f);
	}
	else { debug_path_pos = 0; }
	 // Reset view (Don't use camera for hud)
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glTranslatef(-1, -1, 0);
	glScalef(1/20.0, 1/15.0, 1);
	glTranslatef(0.45*PX/2, 0.45*PX/2, 0);
	 // Draw hud
	if (draw_hud) {
		(*draw_hud)();
	}
	 // Scale view
	if (window_scale > 1.0) {
		glDisable(GL_BLEND);
		glPixelZoom(window_scale, window_scale);
		glCopyPixels(0, 0, 320, 240, GL_COLOR);
		glPixelZoom(1.0, 1.0);
		glEnable(GL_BLEND);
	}
	window->ShowMouseCursor(!trap_cursor);
	draw_latency += frameclock.GetElapsedTime();
	frameclock.Reset();
	if (draw_latency < 0) sf::Sleep(-draw_latency);
	dbg(6, "%f\r", draw_latency);
	if (DEBUG >= 6) fflush(stdout);
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


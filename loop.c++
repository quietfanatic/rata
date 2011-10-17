

void quit_game () {
	window->Close();
	throw 0;
}

void set_video () {
	if (window_fullscreen) {
		window->Create(sf::VideoMode(640, 480, 32), "", sf::Style::Fullscreen);
		window->SetView(window_view);
	}
	else {
		window->Create(sf::VideoMode(320*window_scale, 240*window_scale, 32), "");
		window->SetView(window_view);
	}
	window->UseVerticalSync(true);
	window->SetFramerateLimit(60);
	window->EnableKeyRepeat(false);
	window->ShowMouseCursor(false);
	window->Display();
	lastframe = frameclock.GetElapsedTime();
}


void main_init () {
	 // SFML
	window = new sf::RenderWindow;
	//window = new sf::Image(320, 240);
	set_video();
	 // box2d
	world = new b2World(
		b2Vec2(0.0, -30.0),  // grav
		true  // allow sleep
	);
	world->SetContactListener(new myCL);
	 // Initial room
	//room::testroom.start();	
}



void create_phase () {
	 // We must repeat if an object creates a new object in on_create()
	while (Object* tq = creation_queue) {
		creation_queue = NULL;
		for (Object* next; tq; tq = next) {
			Object* c = tq;
			next = c->next_depth;
			Object** o;
			for (o = &objects_by_depth; *o; o = &(*o)->next_depth) {
				if (obj::def[c->desc->id].depth > obj::def[(*o)->desc->id].depth) {
					c->next_depth = *o;
					*o = c;
					goto done_depth;
				}
			}  // Least deep object
			c->next_depth = NULL;
			*o = c;
			done_depth:
			for (o = &objects_by_order; *o; o = &(*o)->next_order) {
				if (obj::def[c->desc->id].order > obj::def[(*o)->desc->id].order) {
					c->next_order = *o;
					*o = c;
					goto done_order;
				}
			}  // Least deep object
			c->next_order = NULL;
			*o = c;
			done_order:
			c->on_create();
	//		if (c->realtest != 151783) printf("Error: junk object detected\n");
		}
	}
}

void destroy_phase () {
	for (Object** o = &objects_by_depth; *o;) {
		if ((*o)->doomed) {
			*o = (*o)->next_depth;
		}
		else { o = &(*o)->next_depth; }
	}
	for (Object** o = &objects_by_order; *o;) {
		if ((*o)->doomed) {
			Object* doomed = *o;
			doomed->on_destroy();
			*o = doomed->next_order;
			if (doomed->body) {
				world->DestroyBody(doomed->body);
			}
			if (doomed->desc->temp) delete doomed->desc;
			delete doomed;
		}  // Need to ensure we don't leave any dead pointers
		else { o = &(*o)->next_order; }
	}
}


void draw_tiles_back () {
}

void draw_tiles_front () {
}


void draw_phase () {
	Room* rc = room::current;
	if (rata) {
		float focusx = rata->aim_center_x() + cursor.x/2.0;
		float focusy = rata->aim_center_y() + cursor.y/2.0;
		if (focusx < 10) focusx = 10;
		if (focusy < 7.5) focusy = 7.5;
		if (focusx > rc->width-10) focusx = rc->width-10;
		if (focusy > rc->height-7.5) focusy = rc->height-7.5;
		 // To look smooth in a pixelated environment,
		 //  we need a minimum speed.
		if (abs_f(focusx - camera.x) < .25*PX) camera.x = focusx;
		else {
			float newx = (9*camera.x + focusx) / 10;
			if (abs_f(newx - camera.x) < .25*PX) camera.x += .25*PX * sign_f(newx - camera.x);
			else camera.x = newx;
		}
		if (abs_f(focusy - camera.y) < .25*PX) camera.y = focusy;
		else {
			float newy = (9*camera.y + focusy) / 10;
			if (abs_f(newy - camera.y) < .25*PX) camera.y += .25*PX * sign_f(newy - camera.y);
			else camera.y = newy;
		}
		window_view.SetCenter(camera.x, camera.y);

	}
	window->SetView(window_view);
	 // Draw background
	if (rc && rc->bg_index < 0)
		window->Clear(rc->bg_color);
	else if (rc) {
//		draw_image(img::_bgs[rc->bg_index], camera.x, camera.y);
		float w = img::_bgs[rc->bg_index]->sfi.GetWidth()*PX;
		float h = img::_bgs[rc->bg_index]->sfi.GetHeight()*PX;
		float bg_x = mod_f(-camera.x/2, w);
		float bg_y = mod_f(-camera.y/2, h);
		for (float x = bg_x + viewleft(); x < viewright(); x += w)
		for (float y = bg_y + h + viewtop(); y > viewbottom(); y -= h) {
			draw_image(img::_bgs[rc->bg_index], x, y);
		}
	}
	uint minx, miny, maxx, maxy;
	 // Draw back tiles
	if (rc) {
//		if (mapeditor) {
//			minx = miny = 0;
//			maxx = rc->width;
//			maxy = rc->height;
//		}
//		else {
			minx = MAX(0, viewleft());
			miny = MAX(0, viewbottom());
			maxx = MIN(rc->width, ceil(viewright()));
			maxy = MIN(rc->height, ceil(viewtop()));
//		}
//		printf("Drawing tilemap from %d, %d to %d, %d\n", minx, miny, maxx, maxy);
		for (uint x=minx; x < maxx; x++)
		for (uint y=miny; y < maxy; y++) {
		//for (uint x=0; x<width; x++)
		//for (uint y=0; y<height; y++) {
			int tile = rc->tile(x, rc->height-1-y);
			bool flip = (tile < 0);
			if (flip) tile = -tile;
			if (tileinfo[tile].back) {
				//printf("Drawing tile %d at %d, %d\n", tile, x, y);
				draw_image(
					&img::tiles,
					x+.5, y+.5,
					tile, flip
				);
			}
		}
	}
	 // Draw objects
	for (Object* o = objects_by_depth; o; o = o->next_depth) {
		//if (o->realtest != 151783) printf("Error: junk object detected\n");
		dbg(8, "Drawing 0x%08x\n", o);
		o->draw();
		 // Debug draw
		if (debug_mode) {
			if (o->body)
				window->Draw(sf::Shape::Rectangle(
					o->x()-1/16.0, o->y()-1/16.0,
					o->x()+1/16.0, o->y()+1/16.0, sf::Color(255, 0, 0, 127)
				));
		}
	}

	 // Draw front tiles
	if (rc) {
		for (uint x=minx; x < maxx; x++)
		for (uint y=miny; y < maxy; y++) {
		//for (uint x=0; x<width; x++)
		//for (uint y=0; y<height; y++) {
			int tile = rc->tile(x, rc->height-1-y);
			bool flip = (tile < 0);
			if (flip) tile = -tile;
			if (tileinfo[tile].front) {
				//printf("Drawing tile %d at %d, %d\n", tile, x, y);
				draw_image(
					&img::tiles,
					x+.5, y+.5,
					tile, flip
				);
			}
		}
	}
	if (rc && debug_mode) {
		 // Debug draw tilemap
		for (b2Fixture* f = room::tilemap_obj->body->GetFixtureList(); f; f = f->GetNext()) {
			b2EdgeShape* e = (b2EdgeShape*)f->GetShape();
			if (e->m_vertex1.x > viewleft() - 1
			 && e->m_vertex1.x < viewright() + 1
			 && e->m_vertex1.y > viewbottom() - 1
			 && e->m_vertex1.y < viewtop() + 1)
				window->Draw(sf::Shape::Line(
					e->m_vertex1.x, e->m_vertex1.y,
					e->m_vertex2.x, e->m_vertex2.y,
					1.0, sf::Color(0, 255, 0, 127)
				));
			/*window->Draw(sf::Shape::Line(
				e->m_vertex1.x, -e->m_vertex1.y,
				e->m_vertex0.x+3*PX, -e->m_vertex0.y+3*PX,
				1.0, sf::Color(255, 255, 0, 127)
			));
			window->Draw(sf::Shape::Line(
				e->m_vertex3.x-3*PX, -e->m_vertex3.y-3*PX,
				e->m_vertex2.x, -e->m_vertex2.y,
				1.0, sf::Color(0, 0, 255, 127)
			));*/
		}
		 // Debug draw camera
		window->Draw(sf::Shape::Rectangle(
			camera.x-0.2, camera.y-0.2,
			camera.x+0.2, camera.y+0.2,
			sf::Color(255, 0, 0, 127)
		));
	}
	 // Draw lifebar
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
				&img::heart,
				19.5 - (i * 12*PX),
				14.25,
				subimage,
				false, true
			);
		}
	}
	 // Draw screen shade
	if (screen_shade) {
		sf::Shape shade_rect = sf::Shape::Rectangle(
			camera.x*UNPX - 160, -camera.y*UNPX - 120,
			camera.x*UNPX + 160, -camera.y*UNPX + 120,
			screen_shade_color
		);
		shade_rect.SetBlendMode(screen_shade_blend);
		window->Draw(shade_rect);
	}
	 // Draw cursor
	if (rata && rata->state != Rata::dead && rata->state != Rata::dead_air) {
		window->ShowMouseCursor(false);
		draw_image(cursor.img, cursor.x + rata->aim_center_x(), cursor.y + rata->aim_center_y());
	}
	else {
		window->ShowMouseCursor(true);
	}
	 // Draw text message
	if (message) {
		float w = text_width(message_pos);
		float pos = viewwidth()/2 - w*PX/2;
		if (message_pos_next) w += 8;
		else w += 6;
		window->Draw(sf::Shape::Rectangle(
			camera.x - w*PX/2, camera.y - 6.5,
			camera.x + w*PX/2, camera.y - 7.5,
			sf::Color(0, 0, 0, 127)
		));
		char* p;
		for (p = message_pos; message_pos_next ? (p < message_pos_next) : (*p); p++) {
			draw_image(
				&img::font_proportional, pos, 1,
				*p, false, true
			);
			pos += letter_width[(uint8)*p]*PX;
		}
	}
	//sf::Sprite window_s (window);
	//window_s.SetScale(window_scale, window_scale);
	//window->Draw(window_s);
	window->Display();
	float time = frameclock.GetElapsedTime();
	dbg(6, "%f\r", 1/(time - lastframe));
	if (DEBUG >= 6) fflush(stdout);
	lastframe = time;
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
			if (event.Key.Code == sf::Key::Num1) room::file::test1::room.enter();
			if (event.Key.Code == sf::Key::Num2) room::file::test2::room.enter();
			if (event.Key.Code == sf::Key::Num3) room::file::test3::room.enter();
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
			if (rata && rata->state != Rata::dead && rata->state != Rata::dead_air && frame_number > 1) {
				cursor.x += (event.MouseMove.X - window->GetWidth()/2.0)
					* PX * cursor_scale / window_scale;
				cursor.y += -(event.MouseMove.Y - window->GetHeight()/2.0)
					* PX * cursor_scale / window_scale;
				window->SetCursorPosition(window->GetWidth()/2, window->GetHeight()/2);
				if (cursor.x > 19) cursor.x = 19;
				else if (cursor.x < -19) cursor.x = -19;
				if (cursor.y > 14) cursor.y = 14;
				else if (cursor.y < -14) cursor.y = -14;
			}
			else {
				cursor2.x = event.MouseMove.X*PX/window_scale;
				cursor2.y = (window->GetHeight() - event.MouseMove.Y)*PX/window_scale;
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
	for (Object* o = objects_by_order; o; o = o->next_order)
		o->before_move();
	
	if (world) {
		world->Step(1/120.0, 10, 10);
		world->Step(1/120.0, 10, 10);
		//world->Step(1/180.0, 10, 10);
		//world->Step(1/240.0, 10, 10);
	}

	for (Object* o = objects_by_order; o; o = o->next_order)
		o->after_move();
}


void main_loop () {
	window->SetCursorPosition(window->GetWidth()/2, window->GetHeight()/2);
	for (;;) {
		frame_number++;
		create_phase();
		if (!room::transition) draw_phase();
		if (!room::transition) destroy_phase();
		if (!room::transition) input_phase();
		if (!room::transition) move_phase();
		room::transition = false;
		destroy_phase();
	}
}





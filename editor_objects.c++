

struct EditorMenu : Object {
	void draw () {
		draw_rect(
			0,
			45/window_scale,
			60/window_scale,
			43/window_scale,
			sf::Color(31, 31, 31, 127),
			true
		);
	}
};

struct ClickableText : Object {
	typedef void(* command_type )();
	char* message () { return (char*)desc->data; }
	command_type command () { return (command_type)desc->data2; }
	void draw () {
		/*draw_rect(
			desc->x,
			desc->y,
			desc->x + text_width(message())*PX*2/window_scale,
			desc->y-1,
			sf::Color(31, 31, 31, 127),
			true
		);*/
		render_text(message(), desc->x, desc->y, 1, true, 2/window_scale); 
	}
	void before_move () {
		if (click_taken) return;
		if (button[sf::Mouse::Left] == 1)
		if (cursor2.x > desc->x)
		if (cursor2.x < desc->x + text_width(message())*PX*2/window_scale)
		if (cursor2.y < desc->y)
		if (cursor2.y > desc->y - 1) {
			click_taken = true;
			(*command())();
		}
	}
};


struct TilePicker : Object {
	bool resizing;
	void draw () {
		if (room::current) {
			sf::FloatRect vr = window_view.GetRect();
			draw_rect(
				vr.Left, vr.Bottom - 32*PX/window_scale,
				vr.Left + tilepicker_width, vr.Top,
				room::current->bg_color
			);
			draw_rect(
				vr.Left + tilepicker_width,
				vr.Bottom - 32*PX/window_scale,
				vr.Left + tilepicker_width + 4*PX,
				vr.Top,
				sf::Color(31, 31, 31, 127)
			);
				
			for (uint i=0; i < num_tiles; i++) {
				draw_image(
					&img::tiles,
					i % (uint)tilepicker_width + 0.5,
					45/window_scale - 32*PX/window_scale - (i / (uint)tilepicker_width) - 0.5,
					i, flip_tile, true
				);
			}
			window->Draw(sf::Shape::Rectangle(
				vr.Left + (selected_tile % (uint)tilepicker_width),
				vr.Bottom - 32*PX/window_scale - (selected_tile / (uint)tilepicker_width),
				vr.Left + (selected_tile % (uint)tilepicker_width) + 1,
				vr.Bottom - 32*PX/window_scale - (selected_tile / (uint)tilepicker_width) - 1,
				sf::Color(0,0,0,0), 1*PX, sf::Color(255, 255, 255, 127)
			));
		}
	}
	void before_move () {
		if (click_taken) {
			printf("Click taken.\n");
			return;
		}
		 // Flip tile
		if (button[sf::Mouse::Middle] == 1) {
			flip_tile = !flip_tile;
		}
		 // Continue resizing if doing so
		if (resizing && button[sf::Mouse::Left]) {
			click_taken = true;
			if (cursor2.x >= 1)
				tilepicker_width = cursor2.x;
		}
		else {
			resizing = false;
			if (cursor2.x < tilepicker_width) {
				click_taken = true;
				 // Select tile
				if (button[sf::Mouse::Left] == 1 || button[sf::Mouse::Right] == 1) {
					if (cursor2.x < (uint)tilepicker_width) {
						uint clicked_tile = (uint)cursor2.x
						                  + (uint)(45/window_scale-32*PX/window_scale - cursor2.y)
						                  * (uint)tilepicker_width;
						printf("Click: %f, %f -> %d\n", cursor2.x, cursor2.y, clicked_tile);
						if (clicked_tile < num_tiles) {
							selected_tile = clicked_tile;
							printf("Selected %d.\n", selected_tile);
						}
					}
				}
			}
			 // Initiate resize
			else if (cursor2.x < tilepicker_width + 4*PX) {
				if (button[sf::Mouse::Left] == 1) {
					click_taken = true;
					resizing = true;
					printf("Initiating resize of tile picker.\n");
				}
			}
		}

		 // Move View
		if (key[sf::Key::W]) {
			window_view.SetFromRect(sf::FloatRect(
				window_view.GetRect().Left,
				window_view.GetRect().Bottom + 2*PX,
				window_view.GetRect().Right,
				window_view.GetRect().Top + 2*PX
			));
		}
		if (key[sf::Key::A]) {
			window_view.SetFromRect(sf::FloatRect(
				window_view.GetRect().Left - 2*PX,
				window_view.GetRect().Bottom,
				window_view.GetRect().Right - 2*PX,
				window_view.GetRect().Top
			));
		}
		if (key[sf::Key::S]) {
			window_view.SetFromRect(sf::FloatRect(
				window_view.GetRect().Left,
				window_view.GetRect().Bottom - 2*PX,
				window_view.GetRect().Right,
				window_view.GetRect().Top - 2*PX
			));
		}
		if (key[sf::Key::D]) {
			window_view.SetFromRect(sf::FloatRect(
				window_view.GetRect().Left + 2*PX,
				window_view.GetRect().Bottom,
				window_view.GetRect().Right + 2*PX,
				window_view.GetRect().Top
			));
		}
	}
};



struct TilemapEditor : Object {
	void draw () {
		if (room::current) {
			float x = cursor2.x + window_view.GetRect().Left;
			float y = cursor2.y + window_view.GetRect().Top;
			if (x > 0)
			if (x < room::current->width)
			if (y > 0)
			if (y < room::current->height) {
				window->Draw(sf::Shape::Rectangle(
					std::floor(x), std::floor(y),
					(std::floor(x)+1), (std::floor(y)+1),
					sf::Color(0,0,0,0), 1*PX, sf::Color(255, 255, 255, 127)
				));
			}
		}
	}
	void before_move () {
		if (click_taken) return;
		room::Room* rc = room::current;
		float x = cursor2.x + window_view.GetRect().Left;
		float y = cursor2.y + window_view.GetRect().Top;
	//	printf("%f, %f\n", x, y);
		if (rc)
		if (x > 0 && x < rc->width)
		if (y > 0 && y < rc->height) {
			if (button[sf::Mouse::Left]) {
				click_taken = true;
				rc->tiles[(uint)(rc->height - y)*rc->width + (uint)x]
					 = (flip_tile? -selected_tile : selected_tile);
			}
			else if (button[sf::Mouse::Right]) {
				click_taken = true;
				selected_tile = rc->tiles[(uint)(rc->height - y)*rc->width + (uint)x];
				if (selected_tile < 0) {
					flip_tile = true;
					selected_tile = -selected_tile;
				}
				else if (selected_tile > 0) {
					flip_tile = false;
				}
			}
		}
	}
};


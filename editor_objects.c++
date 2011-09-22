

struct ClickableText : Object {
	typedef void(* command_type )();
	char* message () { return (char*)desc->data; }
	command_type command () { return (command_type)desc->data2; }
	void draw () {
		draw_rect(desc->x, desc->y, desc->x + text_width(message())*PX, desc->y-1);
		render_text(message(), desc->x, desc->y); 
	}
};


struct TilePicker : Object {
	bool resizing;
	void draw () {
		if (room::current) {
			sf::FloatRect vr = window_view.GetRect();
			draw_rect(
				vr.Left, vr.Top - 16*PX/window_scale,
				vr.Left + tilepicker_width, vr.Bottom,
				room::current->bg_color
			);
				
			for (uint i=0; i < num_tiles; i++) {
				draw_image(
					&img::tiles,
					i % (uint)tilepicker_width + 0.5,
					45/window_scale - 16*PX/window_scale - (i / (uint)tilepicker_width) - 0.5,
					i, flip_tile, true
				);
			}
			window->Draw(sf::Shape::Rectangle(
				vr.Left + (selected_tile % (uint)tilepicker_width),
				vr.Bottom - 16*PX/window_scale - (selected_tile / (uint)tilepicker_width),
				vr.Left + (selected_tile % (uint)tilepicker_width) + 1,
				vr.Bottom - 16*PX/window_scale - (selected_tile / (uint)tilepicker_width) - 1,
				sf::Color(0,0,0,0), 1*PX, sf::Color(255, 255, 255, 127)
			));
		}
	}
	void before_move () {
		if (click_taken) {
			printf("Click taken.\n");
			return;
		}
		if (button[sf::Mouse::Middle] == 1) {
			flip_tile = !flip_tile;
		}
		if (button[sf::Mouse::Left] == 1) {
			if (cursor2.x < tilepicker_width) {
				click_taken = true;
				if (cursor2.x < (uint)tilepicker_width) {
					uint clicked_tile = (uint)cursor2.x
					                  + (uint)45/window_scale-16*PX/window_scale - cursor2.y
					                  * (uint)tilepicker_width;
					if (clicked_tile < num_tiles) {
						selected_tile = clicked_tile;
						printf("Selected %d.\n", selected_tile);
					}
				}
			}
			else if (cursor2.x < tilepicker_width + 4*PX) {
				resizing = true;
				printf("Initiating resize of tile picker.\n");
			}
		}
		else if (button[sf::Mouse::Left] && resizing && cursor2.x >= 1) {
			click_taken = true;
			tilepicker_width = cursor2.x;
		}
		else {
			resizing = false;
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


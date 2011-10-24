

struct Clickable : Object {
	virtual bool click () { return false; }
	virtual bool drag () { return false; }

	void before_move () {
		if (dragging == this) {
			click_taken = true;
			if (!drag()) dragging = NULL;
		}
		else
		if (!dragging)
		if (!click_taken)
		if (button[sf::Mouse::Left] || button[sf::Mouse::Middle] || button[sf::Mouse::Right])
		if (cursor2.x >= desc->pos.x)
		if (cursor2.y >= desc->pos.y)
		if (cursor2.x <= desc->pos.x + desc->vel.x)
		if (cursor2.y <= desc->pos.y + desc->vel.y)
		if (click())
			click_taken = true;
	}
};




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

struct ClickableText : Clickable {
	typedef void(* command_type )();
	char* message () { return (char*)desc->data; }
	command_type command () { return (command_type)desc->data2; }
	void draw () {
		render_text(message(), desc->pos + Vec(0, 1), 1, false, false, 1, true); 
	}
	bool click () {
		if (button[sf::Mouse::Left] == 1) {
			(*command())();
			return true;
		}
		else return false;
	}
};


struct TilePicker : Clickable {
	float width () { return desc->vel.x - 3.9*PX; }
	void draw () {
		if (room::current) {
			draw_rect(
				viewleft(), viewtop() - 32*PX/window_scale,
				viewleft() + width(), viewbottom(),
				room::current->bg_color
			);
			draw_rect(
				viewleft() + width(),
				viewtop() - 32*PX/window_scale,
				viewleft() + desc->vel.x,
				viewbottom(),
				sf::Color(31, 31, 31, 127)
			);
				
			for (uint i=0; i < num_tiles; i++) {
				draw_image(
					&img::tiles,
					Vec(i % (uint)width() + 0.5,
					    45/window_scale - 32*PX/window_scale - (i / (uint)width()) - 0.5),
					i, flip_tile, true
				);
			}
			window->Draw(sf::Shape::Rectangle(
				viewleft() + (selected_tile % (uint)width()),
				viewtop() - 32*PX/window_scale - (selected_tile / (uint)width()),
				viewleft() + (selected_tile % (uint)width()) + 1,
				viewtop() - 32*PX/window_scale - (selected_tile / (uint)width()) - 1,
				sf::Color(0,0,0,0), 1*PX, sf::Color(255, 255, 255, 127)
			));
		}
	}

	bool click () {
		if (button[sf::Mouse::Middle] == 1) {
			flip_tile = !flip_tile;
			return true;
		}
		if (button[sf::Mouse::Left] == 1 || button[sf::Mouse::Right] == 1) {
			if (cursor2.x < (uint)width()) {
				 // Select tile
				uint clicked_tile = (uint)cursor2.x
				                  + (uint)(45/window_scale-32*PX/window_scale - cursor2.y)
				                  * (uint)width();
				printf("Click: %f, %f -> %d\n", cursor2.x, cursor2.y, clicked_tile);
				if (clicked_tile < num_tiles) {
					selected_tile = clicked_tile;
					printf("Selected %d.\n", selected_tile);
				}
				return true;
			}
			 // Initiate resize
			else {
				dragging = this;
				printf("Initiating resize of tile picker.\n");
				return true;
			}
		}
		else return false;
	}

	bool drag () {
		if (button[sf::Mouse::Left]) {
			if (cursor2.x >= 1+2*PX)
				desc->vel.x = cursor2.x + 2*PX;
			return true;
		}
		else return false;
	}
};



struct TilemapEditor : Clickable {
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
		Clickable::before_move();
	}
	bool click () {
		room::Room* rc = room::current;
		float x = cursor2.x + window_view.GetRect().Left;
		float y = cursor2.y + window_view.GetRect().Top;
		if (button[sf::Mouse::Middle] == 1) {
			flip_tile = !flip_tile;
			return true;
		}
		else if (rc)
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
			return true;
		}
		return false;
	}
};

struct RoomSettings : Clickable {
	void draw () {
		if (room::current) {
			draw_rect(
				viewright() - desc->vel.x, viewtop() - 32*PX/window_scale,
				viewright(), viewbottom(),
				room::current->bg_color
			);
			draw_rect(
				viewright() - desc->vel.x-2*PX,
				viewtop() - 32*PX/window_scale,
				viewright() - desc->vel.x,
				viewbottom(),
				sf::Color(31, 31, 31, 127)
			);
		}
	}
};











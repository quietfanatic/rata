



struct TilePicker : Object {
	bool resizing;
	void draw () {
		if (room::current) {
			sf::FloatRect vr = window_view.GetRect();
			window->Draw(sf::Shape::Rectangle(
				vr.Left, vr.Top + 16/window_scale,
				vr.Left + tilepicker_width*UNPX,
				vr.Bottom,
				room::current->bg_color
			));
			window->Draw(sf::Shape::Rectangle(
				vr.Left + tilepicker_width*UNPX,
				vr.Top + 16/window_scale,
				vr.Left + tilepicker_width*UNPX + 4,
				vr.Bottom,
				sf::Color(31, 31, 31, 127)
			));
				
			for (uint i=0; i < num_tiles; i++) {
				draw_image(
					&img::tiles,
					i % (uint)tilepicker_width + 0.5,
					(720-16)*PX/window_scale - (i / (uint)tilepicker_width) - 0.5,
					i, selected_tile < 0, true
				);
			}
			window->Draw(sf::Shape::Rectangle(
				vr.Left + (selected_tile % (uint)tilepicker_width)*UNPX,
				vr.Top + 16/window_scale + (selected_tile / (uint)tilepicker_width)*UNPX,
				vr.Left + (selected_tile % (uint)tilepicker_width)*UNPX + 1*UNPX,
				vr.Top + 16/window_scale + (selected_tile / (uint)tilepicker_width)*UNPX + 1*UNPX,
				sf::Color(0,0,0,0), 1, sf::Color(255, 255, 255, 127)
			));
		}
	}
	void before_move () {
		if (click_taken) {
			printf("Click taken.\n");
			return;
		}
		if (button[sf::Mouse::Left] == 1) {
			if (cursor2.x < tilepicker_width) {
				click_taken = true;
				if (cursor2.x < (uint)tilepicker_width) {
					uint clicked_tile = (uint)cursor2.x
					                  + (uint)((720-16)*PX/window_scale - cursor2.y)
					                  * (uint)tilepicker_width;
					if (clicked_tile < num_tiles) {
						if (selected_tile < 0) selected_tile = -clicked_tile;
						else selected_tile = clicked_tile;
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
			float x = cursor2.x + window_view.GetRect().Left*PX;
			float y = cursor2.y - window_view.GetRect().Bottom*PX;
			if (x > 0)
			if (x < room::current->width)
			if (y > 0)
			if (y < room::current->height) {
				window->Draw(sf::Shape::Rectangle(
					std::floor(x)*UNPX, -std::floor(y)*UNPX,
					(std::floor(x)+1)*UNPX, -(std::floor(y)+1)*UNPX,
					sf::Color(0,0,0,0), 1, sf::Color(255, 255, 255, 127)
				));
			}
		}
	}
	void before_move () {
		if (click_taken) return;
		click_taken = true;
		room::Room* rc = room::current;
		float x = cursor2.x + window_view.GetRect().Left*PX;
		float y = cursor2.y - window_view.GetRect().Bottom*PX;
	//	printf("%f, %f\n", x, y);
		if (rc && button[sf::Mouse::Left]) {
			if (x > 0 && x < rc->width)
			if (y > 0 && y < rc->height) {
				rc->tiles[(uint)(rc->height - y)*rc->width + (uint)x]
				 = selected_tile;
			}
		}
	}
};


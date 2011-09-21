




struct TilemapEditor : Object {
	void draw () {
		if (room::current) {
			if (cursor2.x > 0 && cursor2.x < room::current->width)
			if (cursor2.y > 0 && cursor2.y < room::current->height) {
				window->Draw(sf::Shape::Rectangle(
					std::floor(cursor2.x)*UNPX, -std::floor(cursor2.y)*UNPX,
					(std::floor(cursor2.x)+1)*UNPX, -(std::floor(cursor2.y)+1)*UNPX,
					sf::Color(0,0,0,0), 1, sf::Color(255, 255, 255, 127)
				));
			}
		}
	}
	void before_move () {
		room::Room* rc = room::current;
		if (rc && button[sf::Mouse::Left]) {
			if (cursor2.x > 0 && cursor2.x < rc->width)
			if (cursor2.y > 0 && cursor2.y < rc->height) {
				rc->tiles[(uint)(rc->height - cursor2.y)*rc->width + (uint)cursor2.x]
				 = selected_tile;
			}
		}
	}
};


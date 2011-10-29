

struct TileLayer : Object { };
struct BulletLayer : Object { };

struct Shade : Object {
	void draw () {
		draw_rect(
			desc->pos.x, desc->pos.y,
			desc->pos.x + desc->vel.x, desc->pos.y + desc->vel.y,
			desc->data, !desc->facing
		);
	}
};


struct Lifebar : Object {
	void draw () {
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
					false, true
				);
			}
		}
	}
};
















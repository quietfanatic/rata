
namespace item {

	struct Equip {
		float ground_accel;
		float ground_decel;
		float max_forward_speed;
		float max_backward_speed;
		float air_accel;
		float max_air_speed;
		float jump_velocity;
		float jump_float_time;
		float damage_mult;
		uint capacity;
	
		img::Image* body;
		img::Image* head;
		img::Image* arm;
		img::Image* hand;
		void (* fire ) ();
	};


	Equip white_dress = {1,1,1,1,1,1,1,1,1, 4,
		&img::dress_body,
		NULL,
		NULL,
		NULL,
		NULL
	};
}










struct FixProp;
struct Actor;
struct Spatial;
struct Object;
struct Walking;
struct Rata;
namespace room { struct Def; }
namespace img { struct Def; }

// MAIN CONTROL LOGIC

void quit_game ();
void main_init ();
void main_loop ();
void set_video ();
void toggle_pause ();


// ROOMS


namespace room {
	struct Def {
		Vec pos;
		uint16 width;
		uint16 height;
		const uint16* tiles;
		uint32 n_neighbors;
		room::Def*const* neighbors;
		uint32 n_walls;
		Circle* walls;
		Line* sides;
		void (& init ) ();
		bool loaded;
		Spatial* contents;

		int16 tile (uint x, uint y) {
			return tiles[y * width + x];
		}
		void receive (Spatial* a);
		void release (Spatial* a);
		void load ();
		void unload ();
		bool is_neighbor (room::Def* r);
		void enter ();
	};
	extern Def def [];

}


// TILE DEFINITIONS

namespace tile {
	const uint max_vertexes = 4;
	struct Def {
		const char* name;
		bool front;
		bool back;
		float friction;
		uint nvertexes;
		Vec vertexes [max_vertexes];
		FixProp* prop;
	};
	extern const Def def [];
}


// LOCAL MAP

namespace map {
	struct Tile {
		uint16 tile1;
		uint16 tile2;
		Tile () :tile1(1), tile2(0) { }
	};

	const uint width = 128;
	const uint height = 128;

	Tile world [height][width];

	struct Pos {
		uint8 x;
		uint8 y;
		
		Pos () { }
		Pos (uint8 x, uint8 y) :x(x % width), y(y % height) { }
		Pos (Vec v) :x((int)floor(v.x) % width), y((int)floor(v.y) % height) { }
		Pos left () const { return Pos(x-1, y); }
		Pos right () const { return Pos(x+1, y); }
		Pos down () const { return Pos(x, y-1); }
		Pos up () const { return Pos(x, y+1); }

		bool operator == (Pos p) { return x == p.x && y == p.y; }
	};

	Tile& at (uint x, uint y) {
		return world[y % height][x % width];
	}
	Tile& at (Pos p) {
		return world[p.y][p.x];
	}

	void debug_print () {
		for (uint y=0; y < height; y++) {
			for (uint x=0; x < width; x++) {
				if (world[height-y-1][x].tile1)
					putchar('#');
				else
					putchar('.');
			}
			putchar('\n');
		}
	}

	void load_room (room::Def*);
	void unload_room (room::Def*);
}


// COLLISIONS

struct FixProp {
	bool is_standable;
	bool stands;
	float damage_factor;
	int touch_damage;
	bool damages_enemies;
} default_FixProp = {true, false, 0.0, 0, false};

namespace cf {
	b2Filter disabled = {0, 0, 0};
	b2Filter rata = {1, 2|4|8|16|32|256, 0};
	b2Filter solid = {2, 1|4|8|16|32|128|256, 0};
	b2Filter bullet = {4, 1|2|8|32, 0};
	b2Filter enemy = {8, 1|2|4|8|32|128|256, 0};
	b2Filter pickup = {16, 1|2|32|128, 0};
	b2Filter movable = {32, 1|2|4|8|16|32|128|256, 0};
	b2Filter scenery = {64, 2, 0};
	b2Filter rata_invincible = {128, 2|8|16|32|256, 0};
	b2Filter sensor = {256, 2|32, 0};
	b2Filter sight_barrier = {512, 0, 0};
};

struct myCL : public b2ContactListener {
	void PreSolve (b2Contact* contact, const b2Manifold* oldmanifold);
	void PostSolve (b2Contact* contact, const b2ContactImpulse* ci);
	void EndContact (b2Contact* contact);
};

void apply_touch_damage (
	Object* a, Object* b,
	FixProp* afp, FixProp* bfp,
	b2Manifold* manifold
);


// FIXTURE HELPERS

b2PolygonShape* make_poly (uint n, Vec* vs, float radius = 0.01);
b2PolygonShape* make_rect (float w, float h, float radius = 0.01);
b2CircleShape* make_circle (float rad, float x = 0, float y = 0);
b2FixtureDef make_fixdef (
	b2Shape* shape, b2Filter filter,
	float friction=0, float restitution=0,
	float density=0, FixProp* prop=NULL,
	bool sensor=false
);


// COLLISION QUERIES

Object* check_area (float l, float t, float r, float b, uint16 cat = 2|32);

struct LineChecker : public b2RayCastCallback {
	Object* owner;
	uint16 mask;
	float frac;
	b2Fixture* hit;
	Vec norm;
	float32 ReportFixture (b2Fixture* fix, const b2Vec2& p, const b2Vec2& n, float32 f);
};

LineChecker check_line (
	Vec from, Vec to,
	uint16 mask = cf::solid.categoryBits,
	Object* owner = NULL
);
bool look_line (
	Vec from, Vec to,
	uint16 mask = cf::solid.categoryBits
);


// ACTOR TYPE INFORMATION
 //   TODO: get rid of this layer
namespace type {
	enum {
		rata,
		tilemap,
		item,
		bullet,
		crate,
		rat,
		patroller,
		flyer,
		heart,
		door,
		bg_color,
		back_tiles,
		bullet_layer,
		front_tiles,
		cursor_layer,
		shade,
		n_types
	};
	struct Def {
		const char* name;
		int32 nfixes;
		const b2FixtureDef* fixdef;
		float depth;
		img::Def* image;
	};
	extern const Def def [n_types];
}

// ACTORS and intermediate classes

struct Actor {  // Basic actor
	int16 type;
	bool active;
	bool awaiting_activation;
	Actor* next_global;
	Actor* next_active;

	Actor (int16 type);
	virtual void load (uint8** p) { }
	virtual void save (uint8** p) { }

	virtual void activate ();
	virtual void deactivate ();

	virtual void before_move () { }
	virtual void after_move () { }
	virtual void draw () { }
	
	bool has_body ();
	void debug_print ();
};

struct Spatial : Actor {  // Actor that occupies a position in a room
	room::Def* loc;
	Spatial* next;
	Spatial* prev;
	Vec pos;

	Spatial (int16 type, room::Def* loc, Vec pos);
	bool in_room (room::Def* r);
	void change_room ();
};

struct Object : Spatial {  // Actor that has a physical body
	Vec vel;
	b2Body* body;
	uint subimage;
	int life;
	int max_life;
	int facing;

	Object (int16 type, room::Def* loc, Vec pos, Vec vel = Vec(0, 0), int facing = 0);
	void activate ();
	void deactivate ();
	void before_move ();
	void after_move ();
	void draw ();

	virtual void damage (int d);
	virtual void heal (int d);
	virtual void kill ();
	virtual char* describe ();

	b2Body* make_body (b2BodyType btype = b2_staticBody, bool bullet=false);
	void impulse (Vec i);
	void mutual_impulse (Object* other, Vec i);
};

struct Walking : Object {  // Actor that walks using friction
	Object*    floor;
	b2Fixture* floor_fix;
	b2Contact* floor_contact;
	Vec        floor_normal;
	b2Body*          friction_body;
	b2FrictionJoint* friction_joint;
	float floor_friction;
	float ideal_xvel;

	Walking (int16 type, room::Def* loc, Vec pos, Vec vel = Vec(0, 0));

	void activate ();
	void deactivate ();
	void before_move ();
};


// MISC SPECIAL ACTORS

struct Tilemap : Object {
	void after_move () { }
	void before_move () { }
	Tilemap (room::Def* def);
};

struct Door : Spatial {
	room::Def* destloc;
	Vec destpos;
	void before_move ();
	Door (room::Def* loc, Vec pos, room::Def* destloc, Vec dest);
};

struct AI;
struct Rat;
struct Patroller;
struct Flyer;



// CHARACTER STATS

struct MoveStats {
	float decel;
	float walk_accel;
	float walk_speed;
	float run_accel;
	float run_speed;
	float crawl_accel;
	float crawl_speed;
	float air_accel;
	float air_speed;
	float jump_vel;
	float float_time;
	//float fall_speed;
};


// ITEM DEFINITIONS

namespace item {
	enum Slot {
		feet,
		body,
		head,
		arm,
		hand,
		num_slots
	};
	struct Def {
		char* name;
		int8 slot;
		int8 otherslot;
		
		img::Def* appearance;
		uint16 world_frame;
		uint16 inventory_frame;
		img::Def* body;
		img::Def* head;
		img::Def* helmet;
		img::Def* arm;
		img::Def* forearm;
		img::Def* hand;
		void (* use ) ();
		void (* stat_mod ) (MoveStats* s);
	};
}

struct Item : Spatial {
	item::Def* def;
	Item (room::Def* loc, Vec pos, item::Def* def);
	void after_move ();
	void draw ();
};



// RATA

struct Rata : Walking {
	 // Character state
	enum State {
		standing = 1,
		walking,
		kneeling,
		crawling,
		falling,
		ouch,
		hurt_air,
		hurt,
		dead_air,
		dead
	};
	uint state;
	struct Pose {
		int8 body;
		int8 head;
		int8 helmet;
		int8 arm;
		int8 forearm;
		int8 hand;
	};
	Pose pose;
	 // Control
	bool auto_control;
	map::Pos destination;
	bool control_left;
	bool control_right;
	bool control_jump;
	bool control_kneel;
	bool control_action;
	bool control_aim;
	bool control_click;
	bool control_goto;
	 // Timers
	int float_frames;
	int recoil_frames;
	int hurt_frames;
	int inv_frames;
	int adrenaline;
	int hurt_direction;
	int hurt_type_0;
	int hurt_type_1;
	 // Aiming
	float min_aim;
	float max_aim;
	bool aiming;
	bool can_see;
	Object* pointed_object;
	float aim_distance;
	float aim_direction;
	 // For animation and movement
	float distance_walked;
	float oldxrel;
	Vec hand_pos;
	uint angle_frame;  // 0=down, 8=up
	float oldyvel;
	float helmet_angle;
	const Vec* sight_points;
	 // Fixtures
	enum {
		fix_feet,
		fix_27,
		fix_25,
		fix_21,
		fix_h7,
		fix_crawl_r,
		fix_crawl_l,
		fix_sensor_21,
		fix_sensor_floor_r,
		fix_sensor_floor_l,
		fix_sensor_block_r,
		fix_sensor_block_l,
		fix_sensor_wall_r,
		fix_sensor_wall_l,
		fix_helmet_stand,
		fix_helmet_kneel,
		fix_helmet_crawl_r,
		fix_helmet_crawl_l,
	};
	int fix_current;
	int fix_old;
	int fix_helmet_current;
	int fix_helmet_old;
	 // Equipment and inventory
	static const uint MAX_INVENTORY = 10;
	Item* inventory [MAX_INVENTORY];
	Item* equipment [item::num_slots];
	 // Actions
	float action_distance;
	void* action_arg;
	enum Action {
		action_equip,
		action_enter,
	};
	int action;
	void propose_action (int act, void* arg, Vec p, float radius);

	 // Easy access to bits
	item::Def* equip_info (uint i);
	bool wearing_helmet ();
	Vec aim_center ();
	Vec cursor_pos ();
	
	void set_fix (int fix);
	void set_helmet (int fix);
	bool check_fix (int fixi);
	bool check_sensor_floor ();
	bool check_sensor_block ();
	bool check_sensor_wall ();
	bool bullet_inv ();

	void update_fixtures ();

	 // Inventory and Equipment
	void pickup_equip (Item* i);
	void unequip_drop (Item* i);

	 // Character stats (affected by items and such)
	MoveStats stats;
	void recalc_stats ();

	 // controls
	void read_controls ();

	 // Actions that can be taken
	void allow_turn ();
	void allow_look ();
	bool allow_aim ();
	bool allow_walk ();
	bool allow_jump ();
	bool allow_kneel ();
	bool allow_crawl ();
	void allow_airmove ();
	void allow_use ();
	void allow_action ();
	void allow_examine ();

	void decrement_counters ();

	void before_move ();
	void after_move ();
	void draw ();

	void damage (int d);
	void kill ();

	uint pose_arm_by_aim ();

	Rata (room::Def* loc, Vec pos, Vec vel = Vec(0, 0), int facing_ = 1);
};


// BULLETS

struct RBullet {
	int power;
	float mass;
	Vec pos0;
	Vec pos1;
	Vec pos2;
	Vec vel;  // Measured in blocks per FRAME not second.
	Object* owner;
	int lifetime;
	RBullet ();
	void move ();
	void draw ();
};
RBullet* fire_rbullet (Object* owner, Vec pos, Vec vel, int power = 48, float mass = 0.2);
RBullet* fire_rbullet_dir (Object* owner, Vec pos, float angle, float vel, int power = 48, float spread = 0.01, float mass = 0.2);
RBullet* fire_rbullet_to (Object* owner, Vec pos, Vec to, float vel, int power = 48, float spread = 0.01, float mass = 0.2);


// GRAPHICS

struct Color {  // Just let me use hexadecimal colors, okay?
	uint32 x;
	Color () { }
	Color (uint32 x) :x(x) { }
	Color (uint32 r, uint32 g, uint32 b, uint32 a) :x(r<<24|g<<16|b<<8|a) { }
	bool visible () const { return (uint8)x; }
	void setGL () const { glColor4ub(x>>24, x>>16, x>>8, x); }
};
void vertex (Vec v);
void draw_image (img::Def* image, Vec p, int sub=0, bool fliph=false, bool flipv=false);
void draw_rect (const Rect& r, Color color = 0x2f2f2f7f);
void draw_line (Vec a, Vec b, Color color = 0xffffff7f);
void draw_circle (Vec p, float r, Color color);


// TEXT

void render_text (char* text, Vec p, uint lines=1, bool small=false, bool border=false, int align=1);
uint text_width (char* s);
uint text_width_small (char* s);


// HUD ELEMENTS

struct Button {
	Vec pos;
	Vec size;
	void (* click )();  // Executed immediately on click
	void (* drag )();  // Executed each frame when dragging.
};
const uint n_pause_buttons = 2;
extern Button pause_buttons [n_pause_buttons];

 // Hud drawing routines
void hud_play ();
void hud_pause ();


// CAMERA ATTENTION POINTS

struct Attention {
	float priority;
	Rect range;
	Attention () :priority(-1/0.0) { }
	Attention (float priority, Rect range) :priority(priority), range(range) { }
};
const uint MAX_ATTENTIONS = 8;
Attention attention [MAX_ATTENTIONS];
void propose_attention (Attention);
void reset_attentions ();


// CAMERA WALLS

Line get_tangent (const Circle& a, const Circle& b);
void build_sides (const room::Def* r);


// CAMERA MANIPULATION

 // Constrain point to walls and a rectangle
Vec constrain (Vec p, const Rect& range);
 // Find optimal point to focus camera on
void get_focus ();
 // Send camera toward focus point
void get_camera ();















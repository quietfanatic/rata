



 // Fixture helper functions
b2PolygonShape* make_poly (uint n, Vec* vs, float radius) {
	b2PolygonShape* r = new b2PolygonShape;
	r->Set((b2Vec2*)vs, n);
	r->m_radius = radius;
	return r;
}

b2PolygonShape* make_rect (float w, float h, float radius) {
	return make_poly(4, (Vec[]){
		Vec(-w/2, 0),
		Vec(w/2, 0),
		Vec(w/2, h),
		Vec(-w/2, h)
	}, radius);
}

b2CircleShape* make_circle (float rad, float x, float y) {
	b2CircleShape* r = new b2CircleShape;
	r->m_radius = rad;
	r->m_p = Vec(x, y);
	return r;
}


b2FixtureDef make_fixdef (b2Shape* shape, b2Filter filter, float friction, float restitution, float density, FixProp* prop, bool sensor) {
	b2FixtureDef r;
	r.shape = shape;
	r.friction = friction;
	r.restitution = restitution;
	r.density = density;
	r.filter = filter;
	r.userData = prop?prop:&default_FixProp;
	r.isSensor = sensor;
	return r;
}

namespace type {

Vec rata_poly_feet [] = {
	Vec(-2.5*PX, 0*PX),
	Vec(2.5*PX, 0*PX),
	Vec(2.5*PX, 1*PX),
	Vec(-2.5*PX, 1*PX)
};
Vec rata_poly_27 [] = {
	Vec(-2.5*PX, 0.1*PX),
	Vec(2.5*PX, 0.1*PX),
	Vec(5.5*PX, 17*PX),
	Vec(5.5*PX, 24*PX),
	Vec(0*PX, 27*PX),
	Vec(-5.5*PX, 24*PX),
	Vec(-5.5*PX, 17*PX)
};
Vec rata_poly_25 [] = {
	Vec(-2.5*PX, 0.1*PX),
	Vec(2.5*PX, 0.1*PX),
	Vec(5.5*PX, 15*PX),
	Vec(5.5*PX, 22*PX),
	Vec(0*PX, 25*PX),
	Vec(-5.5*PX, 22*PX),
	Vec(-5.5*PX, 15*PX)
};
Vec rata_poly_21 [] = {
	Vec(-2.5*PX, 0.1*PX),
	Vec(2.5*PX, 0.1*PX),
	Vec(5.5*PX, 11*PX),
	Vec(5.5*PX, 18*PX),
	Vec(0*PX, 21*PX),
	Vec(-5.5*PX, 18*PX),
	Vec(-5.5*PX, 11*PX)
};
Vec rata_poly_h7 [] = {
	Vec(-8*PX, 0.1*PX),
	Vec(8*PX, 0.1*PX),
	Vec(4*PX, 7*PX),
	Vec(-4*PX, 7*PX),
};
Vec rata_poly_crawl_r [] = {
	Vec(-11*PX, 0.1*PX),
	Vec(6*PX, 0.1*PX),
	Vec(10*PX, 4*PX),
	Vec(10*PX, 11*PX),
	Vec(4*PX, 14*PX),
};
Vec rata_poly_crawl_l [] = {
	Vec(-4*PX, 14*PX),
	Vec(-10*PX, 11*PX),
	Vec(-10*PX, 4*PX),
	Vec(-6*PX, 0.1*PX),
	Vec(11*PX, 0.1*PX),
};
Vec rata_poly_sensor_21 [] = {
	Vec(-2.5*PX, 0.1*PX),
	Vec(2.5*PX, 0.1*PX),
	Vec(2.5*PX, 21*PX),
	Vec(-2.5*PX, 21*PX),
};
Vec rata_poly_sensor_floor_r [] = {
	Vec(4*PX, -4*PX),
	Vec(12*PX, -4*PX),
	Vec(12*PX, 4*PX),
	Vec(4*PX, 4*PX),
};
Vec rata_poly_sensor_floor_l [] = {
	Vec(-4*PX, 4*PX),
	Vec(-12*PX, 4*PX),
	Vec(-12*PX, -4*PX),
	Vec(-4*PX, -4*PX),
};
Vec rata_poly_sensor_block_r [] = {
	Vec(4*PX, 10*PX),
	Vec(12*PX, 10*PX),
	Vec(12*PX, 12*PX),
	Vec(4*PX, 12*PX),
};
Vec rata_poly_sensor_block_l [] = {
	Vec(-4*PX, 12*PX),
	Vec(-12*PX, 12*PX),
	Vec(-12*PX, 10*PX),
	Vec(-4*PX, 10*PX),
};
Vec rata_poly_sensor_wall_r [] = {
	Vec(4*PX, 17*PX),
	Vec(12*PX, 17*PX),
	Vec(12*PX, 19*PX),
	Vec(4*PX, 19*PX),
};
Vec rata_poly_sensor_wall_l [] = {
	Vec(-4*PX, 19*PX),
	Vec(-12*PX, 19*PX),
	Vec(-12*PX, 17*PX),
	Vec(-4*PX, 17*PX),
};

FixProp rata_fixprop = {true, false, 1.0, 0, false};
FixProp rata_fixprop_feet = {false, true, 1.0, 0, false};
FixProp rata_fixprop_helmet = {true, true, 1.0, 0, false};
b2FixtureDef rata_fixes [] = {
	make_fixdef(make_poly(4, rata_poly_feet), cf::disabled, 0, 0, 1.0, &rata_fixprop_feet, false),
	make_fixdef(make_poly(7, rata_poly_27), cf::disabled, 0, 0, 1.0, &rata_fixprop, false),
	make_fixdef(make_poly(7, rata_poly_25), cf::disabled, 0, 0, 1.0, &rata_fixprop, false),
	make_fixdef(make_poly(7, rata_poly_21), cf::disabled, 0, 0, 1.0, &rata_fixprop, false),
	make_fixdef(make_poly(4, rata_poly_h7), cf::disabled, 0, 0, 1.0, &rata_fixprop, false),
	make_fixdef(make_poly(5, rata_poly_crawl_r), cf::disabled, 0, 0, 1.0, &rata_fixprop, false),
	make_fixdef(make_poly(5, rata_poly_crawl_l), cf::disabled, 0, 0, 1.0, &rata_fixprop, false),
	make_fixdef(make_poly(4, rata_poly_sensor_21), cf::sensor, 0, 0, 0.0, &rata_fixprop, true),
	make_fixdef(make_poly(4, rata_poly_sensor_floor_r), cf::sensor, 0, 0, 0.0, &rata_fixprop, true),
	make_fixdef(make_poly(4, rata_poly_sensor_floor_l), cf::sensor, 0, 0, 0.0, &rata_fixprop, true),
	make_fixdef(make_poly(4, rata_poly_sensor_block_r), cf::sensor, 0, 0, 0.0, &rata_fixprop, true),
	make_fixdef(make_poly(4, rata_poly_sensor_block_l), cf::sensor, 0, 0, 0.0, &rata_fixprop, true),
	make_fixdef(make_poly(4, rata_poly_sensor_wall_r), cf::sensor, 0, 0, 0.0, &rata_fixprop, true),
	make_fixdef(make_poly(4, rata_poly_sensor_wall_l), cf::sensor, 0, 0, 0.0, &rata_fixprop, true),
	make_fixdef(make_circle(7*PX, 0*PX, 23.5*PX), cf::disabled, 0, 0, 0.0, &rata_fixprop_helmet, false),
	make_fixdef(make_circle(7*PX, 0*PX, 16.5*PX), cf::disabled, 0, 0, 0.0, &rata_fixprop_helmet, false),
	make_fixdef(make_circle(7*PX, 4.5*PX, 11.5*PX), cf::disabled, 0, 0, 0.0, &rata_fixprop_helmet, false),
	make_fixdef(make_circle(7*PX, -4.5*PX, 11.5*PX), cf::disabled, 0, 0, 0.0, &rata_fixprop_helmet, false)
};

FixProp bullet_fixprop = {false, false, 0.0, 48, true};
b2FixtureDef bullet_fix = make_fixdef(make_circle(0.05), cf::bullet, 0, 0.8, 10.0, &bullet_fixprop);
FixProp rat_fixprop = {true, true, 1.0, 12, false};
b2FixtureDef rat_fix = make_fixdef(make_rect(12*PX, 5*PX), cf::enemy, 0, 0, 4.0, &rat_fixprop);
FixProp crate_fixprop = {true, false, 1.0, 0, false};
b2FixtureDef crate_fix = make_fixdef(make_rect(1, 1, 0.005), cf::movable, 0.4, 0, 4.0, &crate_fixprop);
b2FixtureDef mousehole_fix = make_fixdef(make_rect(14*PX, 10*PX), cf::scenery, 0, 0, 0);
FixProp patroller_fixprop = {true, true, 1.0, 0, false};
b2FixtureDef patroller_fixes [] = {
	make_fixdef(make_rect(14*PX, 12*PX), cf::enemy, 0, 0.1, 6.0, &patroller_fixprop),
};
FixProp flyer_fixprop = {false, false, 1.0, 12, false};
b2FixtureDef flyer_fix = make_fixdef(make_circle(6*PX), cf::enemy, 0, 0.6, 6.0, &flyer_fixprop);
FixProp heart_fixprop = {false, false, 0.0, 0, false};
b2FixtureDef heart_fix = make_fixdef(make_rect(0.5, 0.5), cf::pickup, 0.8, 0, 0.1, &heart_fixprop);

const Def def [n_types] = {

	{"Rata", 18, rata_fixes, 10, NULL},
	{"Tilemap", 0, NULL, 0, NULL},
	{"Item", -1, NULL, -5, NULL},
	{"Bullet", 1, &bullet_fix, -10, NULL},
	{"Crate", 1, &crate_fix, 0, img::crate},
	{"Rat", 1, &rat_fix, 15, img::rat},
	{"Patroller", 1, patroller_fixes, 20, img::patroller},
	{"Flyer", 1, &flyer_fix, 20, img::flyer},
	{"Heart", 1, &heart_fix, -20, img::heart},
	{"Door", -1, NULL, -100,  NULL},
	{"BG Color", -1, NULL, 1000, NULL},
	{"Back Tiles", -1, NULL, 500, NULL},
	{"Bullet Layer", -1, NULL, -200, NULL},
	{"Front Tiles", -1, NULL, -500, NULL},
	{"Cursor Layer", -1, NULL, -2000, NULL},
	{"Shade", -1, NULL, -3000, NULL},
//	{"Solid Object", -1, NULL, 0, -1},
//	{"Tilemap", 0, NULL, 0, -1},
//	{"Mousehole", 1, &mousehole_fix, 50, img::mousehole},
//	{"Hit Effect", -1, NULL, -90, -1},
};

}

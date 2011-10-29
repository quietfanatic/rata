


namespace pose {


enum Common {
	a0,
	a23,
	a45,
	a68,
	a90,
	a113,
	a135,
	a158,
	a180,
	a203,
	a225,
	a248,
	a270,
	a293,
	a315,
	a338
};


struct Body {
	enum Pose {
		stand,
		walk,
		tiptoe,
		kneel,
		crawl,
		crawl2,
		sit,
		hurtbk,
		laybk,
		hurtfd,
		layfd,
		grab,
		num
	};
	Vec arm; Vec head;
	Body (float armx, float army, float headx, float heady)
	 :arm(Vec(armx*PX, army*PX)), head(Vec(headx*PX, heady*PX)) { }
};

const Body body [Body::num] = {
	Body( -1, 14,    0, 16 ),  // stand
	Body( -1, 13,    0, 15 ),  // walk
	Body( -1, 15,    0, 17 ),  // tiptoe
	Body( -1,  8,    0, 10 ),  // kneel
	Body(  1,  4,    3,  5 ),  // crawl
	Body(  1,  3,    3,  4 ),  // crawl2
	Body( -1,  8,    0,  9 ),  // sit
	Body( -1, 14,    0, 16 ),  // hurtbk
	Body( -1,  1,   -3,  2 ),  // laybk
	Body(  0,  0,    0,  0 ),  // hurtfd
	Body(  0,  0,    0,  0 ),  // layfd
	Body(  0,  0,    0,  0 ),  // grab
};


struct Head {
	enum Pose {
		stand_23,
		stand_45,
		stand_68,
		stand_90,
		stand_135,
		stand_158,
        walk_23,
		walk_45,
		walk_68,
		walk_90,
		walk_135,
		walk_158,
        crawl,
		hurtbk,
		hurtfd,
		laybk,
		layfd,
		num
	};
	static const uint angle_stand [9];
	static const uint angle_walk [9];
	Vec helmet; float helmeta; uint8 helmetf;
	Head (float x, float y, float a, uint8 f)
	 :helmet(Vec(x*PX, y*PX)), helmeta(a), helmetf(f) { }
};

const Head head [Head::num] = {
	Head( -0.5, 6.5, -pi/18, 0),  // stand_23
	Head( -0.5, 7.5,      0, 1),  // stand_45
	Head( -0.5, 7.5,  pi/18, 2),  // stand_68
	Head( -0.5, 7.5,  pi/18, 2),  // stand_90
	Head( -0.5, 7.5,  pi/ 8, 3),  // stand_135
	Head( -1.5, 7.5,  pi/ 4, 4),  // stand_158
	Head( -0.5, 6.5, -pi/18, 0),  // walk_23
	Head( -0.5, 7.5,      0, 1),  // walk_45
	Head( -0.5, 7.5,  pi/18, 2),  // walk_68
	Head( -0.5, 7.5,  pi/18, 2),  // walk_90
	Head( -0.5, 7.5,  pi/ 8, 3),  // walk_135
	Head( -1.5, 7.5,  pi/ 4, 4),  // walk_158
	Head(  0.5, 6.5,      0, 1),  // crawl
	Head( -0.5, 6.5, -pi/18, 0),  // hurtbk
	Head( -0.5, 6.5,      0, 0),  // hurtfd
	Head( -9.5, 0.5, -pi/ 2, 5),  // laybk
	Head(  0.0, 0.0,      0, 0),  // layfd
};
const uint Head::angle_stand [9] = {
	stand_23, stand_23, stand_45, stand_68, stand_90, stand_90, stand_135, stand_158, stand_158
};
const uint Head::angle_walk [9] = {
	 walk_23,  walk_23,  walk_45,  walk_68,  walk_90,  walk_90,  walk_135,  walk_158,  walk_158
};



struct Arm {
	static const int angle_far [9];
	static const int angle_mid [9];
	static const int angle_near [9];
	Vec forearm;
	Arm (float x, float y) :forearm(Vec(x*PX, y*PX)) { }
};

const Arm arm [9] = {
	Arm(0, -3),
	Arm(1, -3),
	Arm(2, -2),
	Arm(3, -1),
	Arm(3,  0),
	Arm(3,  1),
	Arm(2,  2),
	Arm(1,  3),
	Arm(0,  3),
};

const int Arm::angle_far  [9]={ a0,  a23,  a45,  a68, a90, a113, a135, a158, a180 };
const int Arm::angle_mid  [9]={ a0, -a23,   a0,  a23, a45,  a68,  a90, a113, a135 };
const int Arm::angle_near [9]={ a0, -a23, -a45, -a23,  a0,  a23,  a45,  a68,  a90 };



namespace forearm {


const uint num = 9;
enum Pose     {     a0,    a23,    a45,    a68,    a90,   a113,   a135,   a158,   a180 };
float handx[]={   0*PX,   2*PX,   3*PX,   4*PX,   5*PX,   4*PX,   3*PX,   2*PX,   0*PX };
float handy[]={  -5*PX,  -4*PX,  -3*PX,  -2*PX,   0*PX,   2*PX,   3*PX,   4*PX,   5*PX };


}
namespace hand {


const uint num = 10;
enum Pose     {     a0,    a23,    a45,    a68,    a90,   a113,   a135,   a158,   a180,
                  a203,   a225,   a248,   a270,   a293,   a315,   a338, inside,  front, };

}


}












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
};

const Body body [Body::num] = {
	{{-1*PX, 14*PX}, { 0*PX, 16*PX}},  // stand
	{{-1*PX, 13*PX}, { 0*PX, 15*PX}},  // walk
	{{-1*PX, 15*PX}, { 0*PX, 17*PX}},  // tiptoe
	{{-1*PX,  8*PX}, { 0*PX, 10*PX}},  // kneel
	{{ 1*PX,  4*PX}, { 3*PX,  5*PX}},  // crawl
	{{ 1*PX,  3*PX}, { 3*PX,  4*PX}},  // crawl2
	{{-1*PX,  8*PX}, { 0*PX,  9*PX}},  // sit
	{{-1*PX, 14*PX}, { 0*PX, 16*PX}},  // hurtbk
	{{-1*PX,  1*PX}, {-3*PX,  2*PX}},  // laybk
	{{ 0*PX,  0*PX}, { 0*PX,  0*PX}},  // hurtfd
	{{ 0*PX,  0*PX}, { 0*PX,  0*PX}},  // layfd
	{{ 0*PX,  0*PX}, { 0*PX,  0*PX}},  // grab
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
};

const Head head [Head::num] = {
	{{-0.5*PX, 6.5*PX}, -pi/18, 0},  // stand_23
	{{-0.5*PX, 7.5*PX},      0, 1},  // stand_45
	{{-0.5*PX, 7.5*PX},  pi/18, 2},  // stand_68
	{{-0.5*PX, 7.5*PX},  pi/18, 2},  // stand_90
	{{-0.5*PX, 7.5*PX},  pi/ 8, 3},  // stand_135
	{{-1.5*PX, 7.5*PX},  pi/ 4, 4},  // stand_158
	{{-0.5*PX, 6.5*PX}, -pi/18, 0},  // walk_23
	{{-0.5*PX, 7.5*PX},      0, 1},  // walk_45
	{{-0.5*PX, 7.5*PX},  pi/18, 2},  // walk_68
	{{-0.5*PX, 7.5*PX},  pi/18, 2},  // walk_90
	{{-0.5*PX, 7.5*PX},  pi/ 8, 3},  // walk_135
	{{-1.5*PX, 7.5*PX},  pi/ 4, 4},  // walk_158
	{{ 0.5*PX, 6.5*PX},      0, 1},  // crawl
	{{-0.5*PX, 6.5*PX}, -pi/18, 0},  // hurtbk
	{{-0.5*PX, 6.5*PX},      0, 0},  // hurtfd
	{{-9.5*PX, 0.5*PX}, -pi/ 2, 5},  // laybk
	{{ 0.0*PX, 0.0*PX},      0, 0},  // layfd
};
const uint Head::angle_stand [9] = {
	stand_23, stand_23, stand_45, stand_68, stand_90, stand_90, stand_135, stand_158, stand_158
};
const uint Head::angle_walk [9] = {
	 walk_23,  walk_23,  walk_45,  walk_68,  walk_90,  walk_90,  walk_135,  walk_158,  walk_158
};



struct Arm {
	static const uint num = 9;
	static const int angle_far [9];
	static const int angle_mid [9];
	static const int angle_near [9];
	Vec forearm;
};

const Arm arm [Arm::num] = {
	{{0*PX, -3*PX}},
	{{1*PX, -3*PX}},
	{{2*PX, -2*PX}},
	{{3*PX, -1*PX}},
	{{3*PX,  0*PX}},
	{{3*PX,  1*PX}},
	{{2*PX,  2*PX}},
	{{1*PX,  3*PX}},
	{{0*PX,  3*PX}},
};

const int Arm::angle_far  [9]={ a0,  a23,  a45,  a68, a90, a113, a135, a158, a180 };
const int Arm::angle_mid  [9]={ a0, -a23,   a0,  a23, a45,  a68,  a90, a113, a135 };
const int Arm::angle_near [9]={ a0, -a23, -a45, -a23,  a0,  a23,  a45,  a68,  a90 };



struct Forearm {
	static const uint num = 9;
	Vec hand;
};

const Forearm forearm [Forearm::num] = {
	{{0*PX, -5*PX}},
	{{2*PX, -4*PX}},
	{{3*PX, -3*PX}},
	{{4*PX, -2*PX}},
	{{5*PX,  0*PX}},
	{{4*PX,  2*PX}},
	{{3*PX,  3*PX}},
	{{2*PX,  4*PX}},
	{{0*PX,  5*PX}},
};


struct Hand {
	static const uint num = 18;
	enum Pose {
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
		a338,
		inside,
		front
	};
};


}









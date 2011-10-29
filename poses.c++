


namespace pose {




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


namespace head {


const uint num = 16;
enum Pose        { stand_23, stand_45, stand_68, stand_90,stand_135,stand_158,
                    walk_23,  walk_45,  walk_68,  walk_90, walk_135, walk_158,
                      crawl,   hurtbk,   hurtfd,    laybk,    layfd,
};
float helmetx []={  -0.5*PX,  -0.5*PX,  -0.5*PX,  -0.5*PX,  -0.5*PX,  -1.5*PX,
                    -0.5*PX,  -0.5*PX,  -0.5*PX,  -0.5*PX,  -0.5*PX,  -1.5*PX,
                     0.5*PX,  -0.5*PX,  -0.5*PX,  -9.5*PX,     0*PX,
};
float helmety []={   6.5*PX,   7.5*PX,   7.5*PX,   7.5*PX,   7.5*PX,   7.5*PX,
                     6.5*PX,   7.5*PX,   7.5*PX,   7.5*PX,   7.5*PX,   7.5*PX,
                     6.5*PX,   6.5*PX,   6.5*PX,   0.5*PX,     0*PX,
};
float helmeta []={ -M_PI/18,        0,  M_PI/18,  M_PI/18,   M_PI/8,   M_PI/4,
                   -M_PI/18,        0,  M_PI/18,  M_PI/18,   M_PI/8,   M_PI/4,
                          0, -M_PI/18,        0,        0,        0,
};
uint8 helmetf []={        0,        1,        2,        2,        3,        4,
                          0,        1,        2,        2,        3,        4,
                          1,        0,        0,        5,        0,
};
uint angle_stand [9] = {
	stand_23, stand_23, stand_45, stand_68, stand_90, stand_90, stand_135, stand_158, stand_158
};
uint angle_walk [9] = {
	 walk_23,  walk_23,  walk_45,  walk_68,  walk_90,  walk_90,  walk_135,  walk_158,  walk_158
};


}
namespace arm {


const uint num = 9;
enum Pose        {     a0,    a23,    a45,    a68,    a90,   a113,   a135,   a158,   a180 };
float forearmx[]={   0*PX,   1*PX,   2*PX,   3*PX,   3*PX,   3*PX,   2*PX,   1*PX,   0*PX };
float forearmy[]={  -3*PX,  -3*PX,  -2*PX,  -1*PX,   0*PX,   1*PX,   2*PX,   3*PX,   3*PX };

int angle_far  [9]={ a0,  a23,  a45,  a68, a90, a113, a135, a158, a180 };
int angle_mid  [9]={ a0, -a23,   a0,  a23, a45,  a68,  a90, a113, a135 };
int angle_near [9]={ a0, -a23, -a45, -a23,  a0,  a23,  a45,  a68,  a90 };

}
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












namespace pose {

	namespace body {
const uint num = 12;
enum Pose     {  stand,   walk, tiptoe,  kneel,  crawl, crawl2,    sit, hurtbk,  laybk, hurtfd,  layfd,   grab };
float armx []={  -1*PX,  -1*PX,  -1*PX,  -1*PX,   0*PX,   0*PX,  -1*PX,  -1*PX,  -5*PX,   0*PX,   0*PX,   0*PX };
float army []={  14*PX,  14*PX,  15*PX,   8*PX,   0*PX,   0*PX,   7*PX,  12*PX,   1*PX,   0*PX,   0*PX,   0*PX };
float headx[]={   0*PX,   0*PX,   0*PX,   0*PX,   0*PX,   0*PX,   0*PX,   0*PX,  -11*PX,  0*PX,   0*PX,   0*PX };
float heady[]={  16*PX,  16*PX,  17*PX,  10*PX,   0*PX,   0*PX,   9*PX,  14*PX,  -2*PX,   0*PX,   0*PX,   0*PX };
	}
	namespace head {
		const uint num = 16;
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
			hurtbk,
			hurtfd,
			laybk,
			layfd,
		};
		uint angle_stand [] = {
			stand_23, // 0
			stand_23, // 23
			stand_45, // 45
			stand_68, // 68
			stand_90, // 90
			stand_90, // 113
			stand_135, // 135
			stand_158, // 158
			stand_158 // 180
		};
		uint angle_walk [] = {
			walk_23, // 0
			walk_23, // 23
			walk_45, // 45
			walk_68, // 68
			walk_90, // 90
			walk_90, // 113
			walk_135, // 135
			walk_158, // 158
			walk_158 // 180
		};
	}
	namespace arm {
const uint num = 18;
enum Pose     {     e0,    e23,    e45,    e68,    e90,   e113,   e135,   e158,   e180,
                    m0,    m23,    m45,    m68,    m90,   m113,   m135,   m158,   m180
};
float handx[]={   0*PX,   3*PX,   5*PX,   7*PX,   8*PX,   7*PX,   5*PX,   3*PX,   0*PX,
                 -1*PX,   1*PX,   3*PX,   5*PX,   6*PX,   6*PX,   5*PX,   4*PX,   2*PX
};
float handy[]={  -8*PX,  -7*PX,  -5*PX,  -3*PX,   0*PX,   3*PX,   5*PX,   7*PX,   8*PX,
                 -8*PX,  -6*PX,  -5*PX,  -4*PX,  -2*PX,   1*PX,   3*PX,   5*PX,   6*PX
};
		uint angle_e [] = { e0, e23, e45, e68, e90, e113, e135, e158, e180 };
		uint angle_m [] = { m0, m23, m45, m68, m90, m113, m135, m158, m180 };
	}

}












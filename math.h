

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define SWAP(a, b) {typeof(a) _t = b; b = a; a = _t;}
static inline float mod_f (float a, float b) {
	return a >= 0 ? a - b*(double)(int)(a/b)
	              : a - b*(double)(int)(a/b);
}
static inline float abs_f (float x) { return x>0 ? x : -x; }
static inline int sign_f (float x) { return (x>0)-(x<0); }

static inline float dither (float c, float err) {
	return c - err + rand()*2.0*err/RAND_MAX;
}

static inline float flip_angle (float a) {
	return a<0 ? -pi-a : pi-a;
}
static inline bool gt_angle (float a, float b) {
	if (b < 0)
		return a > b && a < b+pi;
	else return a > b || a < b-pi;
}

static uint8 get_angle_frame (float a) {
	if (a != a) return 4;
	if (gt_angle(a, pi/2)) {
		a = flip_angle(a);
	}
	return
	  a > pi * +7/16.0 ? 8
	: a > pi * +5/16.0 ? 7
	: a > pi * +3/16.0 ? 6
	: a > pi * +1/16.0 ? 5
	: a > pi * -1/16.0 ? 4
	: a > pi * -3/16.0 ? 3
	: a > pi * -5/16.0 ? 2
	: a > pi * -7/16.0 ? 1
	:                    0;
}







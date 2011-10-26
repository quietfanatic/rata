

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
	return a<0 ? -M_PI-a : M_PI-a;
}
static inline bool gt_angle (float a, float b) {
	if (b < 0)
		return a > b && a < b+M_PI;
	else return a > b || a < b-M_PI;
}




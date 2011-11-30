

 // GENERIC
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define SWAP(a, b) {typeof(a) _t = b; b = a; a = _t;}
 // FLOAT
float mod_f (float a, float b) {
	return a >= 0 ? a - b*(double)(int)(a/b)
	              : a - b*(double)(int)(a/b);
}
float abs_f (float x) { return x>0 ? x : -x; }
int sign_f (float x) { return (x>0)-(x<0); }

float dither (float c, float err) {
	return c - err + rand()*2.0*err/RAND_MAX;
}

 // ANGLES
float flip_angle (float a) {
	return a<0 ? -pi-a : pi-a;
}
bool gt_angle (float a, float b) {
	if (b < 0)
		return a > b && a < b+pi;
	else return a > b || a < b-pi;
}
 // Using angles in animation
uint8 get_angle_frame (float a) {
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

 // VECTORS

 // Our wheels are better.
struct Vec {
	float x;
	float y;
};

const Vec nanvec = {0/0.0, 0/0.0};
#define vec(x, y) ((Vec){x, y})
Vec b2vec (b2Vec2 v) { return vec(v.x, v.y); }
b2Vec2 vecb2 (Vec v) { return b2Vec2(v.x, v.y); }
sf::Vector2<float> vecsf (Vec v) { return sf::Vector2<float>(v.x, v.y); }

Vec operator - (Vec a) { return vec(-a.x, -a.y); }
Vec operator + (Vec a, Vec b) { return vec(a.x+b.x, a.y+b.y); }
Vec operator - (Vec a, Vec b) { return vec(a.x-b.x, a.y-b.y); }
Vec operator * (Vec a, float b) { return vec(a.x*b, a.y*b); }
Vec operator * (float a, Vec b) { return vec(a*b.x, a*b.y); }
Vec operator / (Vec a,  float b) { return vec(a.x/b, a.y/b); }
Vec operator += (Vec& a, Vec b) { return a = a + b; }
Vec operator -= (Vec& a, Vec b) { return a = a - b; }
Vec operator *= (Vec& a, float b) { return a = a * b; }
Vec operator /= (Vec& a, float b) { return a = a / b; }
bool operator == (Vec a, Vec b) { return a.x==b.x && a.y==b.y; }
bool operator != (Vec a, Vec b) { return a.x!=b.x || a.y!=b.y; }
Vec scalex (Vec a, float b) { return vec(a.x * b, a.y); }
Vec scaley (Vec a, float b) { return vec(a.x, a.y * b); }
float mag2 (Vec a) { return a.x*a.x+a.y*a.y; }
float mag (Vec a) { return sqrt(mag2(a)); }
float dot (Vec a, Vec b) { return a.x*b.x + a.y*b.y; }
float ang (Vec a) { return atan2(a.y, a.x); }
float slope (Vec a) { return a.y / a.x; }
float liney (Vec a, Vec b, float x) {
	return a.y + slope(b - a) * (x - a.x);
}
float linex (Vec a, Vec b, float y) {
	return a.x + (y - a.y) / slope(b - a);
}
Vec norm (Vec a) { return a / mag(a); }
Vec rotcw (Vec a) { return vec(-a.y, a.x); }
Vec rotccw (Vec a) { return vec(a.y, -a.x); }

Vec polar (float r, float a) { return r*vec(cos(a), sin(a)); }

bool defined (float a) { return a==a; }
bool defined (Vec a) { return defined(a.x) && defined(a.y); }


// GEOMETRY

bool in_rect (Vec p, Vec a, Vec b) {
	return p.x >= a.x
	    && p.x <= b.x
	    && p.y >= a.y
	    && p.y <= b.y;
}

bool across_line (Vec p, Vec a, Vec b) {
	if (a.x == b.x)
		return (a.y < b.y) ? p.x < a.x
		                   : p.x > a.x;
	float y = liney(a, b, p.x);
	return (a.x < b.x) ? y < p.y
	                   : y > p.y;
}

Vec uncross_line (Vec p, Vec a, Vec b) {
	// To get the intersection of two lines, use
	// ay + asx = by + bsx (y is y-intercept and s is slope)
	// x = (by - ay) / (as - bs)
	// y = liney(a, b, x)
	if (a.x == b.x)
		return vec(a.x, p.y);
	if (a.y == b.y)
		return vec(p.x, a.y);
	float x = (liney(a, b, 0) - liney(p, p + rotcw(b - a), 0))
	        / (slope(rotcw(b - a)) - slope(b - a));
//	printf("x = (%f-%f)/(%f-%f) = %f\n",
//		liney(a, b, 0), liney(p, p + rotcw(b - a), 0),
//		slope(rotcw(b - a)), slope(b - a),
//		x
//	);
	return vec(x, liney(a, b, x));
}









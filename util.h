

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MOD(a, b) ((a) - (b) * (double)(int)((a)/(b)))
#define SWAP(a, b) {typeof(a) _t = b; b = a; a = _t;}

#define coords2sf(x, y) (x)/PX, (-(y))/PX

#define ID(x) (x)


#define UNPX (1.0/PX)

// Quick math assuming no negative numbers.
//#define round(x) ((int)((x)+0.5))
//#define floor(x) ((int)(x))

sf::Sprite drawing_sprite;
void draw_image_sub (img::Image& img, float x, float y, int l, int t, int r, int b, bool flip=false, bool cam=false) {
	if (!cam)
	if (x < camera.x - 10 - (r-l)
	 || y < camera.y - 7.5 - (b-t)
	 || x > camera.x + 10 + (r-l)
	 || y > camera.y + 7.5 + (b-t)) return;
	
	sf::IntRect sr (l, t, r, b);
	drawing_sprite.SetImage(img.sfi);
	drawing_sprite.SetSubRect(sr);
	if (!cam) {
		if (flip)
			drawing_sprite.SetPosition(round((x)*UNPX - ((r - l) - img.x)), round((-y)*UNPX -  img.y));
		else
			drawing_sprite.SetPosition(round((x)*UNPX - img.x), round((-y)*UNPX - img.y));
	}
	else {
		if (flip)
			drawing_sprite.SetPosition((camera.x+x-10)*UNPX - ((r - l) - img.x), (-(camera.y+y-7.5))*UNPX - img.y);
		else
			drawing_sprite.SetPosition((camera.x+x-10)*UNPX - img.x, (-(camera.y+y-7.5))*UNPX - img.y);
	}
	drawing_sprite.FlipX(flip);
	window->Draw(drawing_sprite);
};

void draw_image (img::Image& img, float x, float y, bool flip=false, bool cam=false) {
	return draw_image_sub(img, x, y, 0, 0, img.sfi.GetWidth(), img.sfi.GetHeight(), flip, cam);
};

inline float abs_f (float x) { return x>0 ? x : -x; }
inline int sign_f (float x) { return (x>0)-(x<0); }



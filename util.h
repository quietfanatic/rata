

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MOD(a, b) ((a) - (b) * (double)(int)((a)/(b)))
#ifdef MAPEDITOR
	#define SWAP(a, b) {decltype(a) _t = b; b = a; a = _t;}
#else
	#define SWAP(a, b) {typeof(a) _t = b; b = a; a = _t;}
#endif

#define coords2sf(x, y) (x)/PX, (-(y))/PX

#define ID(x) (x)


#define UNPX (1.0/PX)

// Quick math assuming no negative numbers.
//#define round(x) ((int)((x)+0.5))
//#define floor(x) ((int)(x))

sf::Sprite drawing_sprite;
void draw_image (img::Image* img, float x, float y, int sub=0, bool flip=false, bool cam=false) {
	//if (!cam)
	//if (x < camera.x - 10 - img->w
	// || y < camera.y - 7.5 - img->h
	// || x > camera.x + 10 + img->w
	// || y > camera.y + 7.5 + img->h) return;
	uint iw = img->sfi.GetWidth();
	uint ih = img->sfi.GetHeight();
	sub %= img->numsubs();
	
	sf::IntRect sr;
	if (img->w == 0 || img->h == 0)
		sr = sf::IntRect(0, 0, iw, ih);
	else
		sr = sf::IntRect(
			sub % (iw/img->w) * img->w,
			sub / (iw/img->w) * img->h,
			sub % (iw/img->w) * img->w + img->w,
			sub / (iw/img->w) * img->h + img->h
		);
	drawing_sprite.SetImage(img->sfi);
	drawing_sprite.SetSubRect(sr);
	drawing_sprite.FlipX(flip);
	uint xpos = flip && img->w ? img->w - img->x : img->x;
	if (!cam)
		drawing_sprite.SetPosition(round((x)*UNPX - xpos), round((-y)*UNPX - img->y));
	else
		drawing_sprite.SetPosition(
			window_view.GetRect().Left + x*UNPX - xpos,
			window_view.GetRect().Bottom - y*UNPX - img->y
		);
	window->Draw(drawing_sprite);
};
void draw_rect (float l, float t, float r, float b, sf::Color color=sf::Color(31, 31, 31, 127), bool cam=false) {
	sf::FloatRect vr = window_view.GetRect();
	if (cam)
		window->Draw(sf::Shape::Rectangle(
			vr.Left + l*PX, vr.Bottom - t*PX,
			vr.Left + r*PX, vr.Bottom - b*PX,
			color
		));
	else
		window->Draw(sf::Shape::Rectangle(
			l*PX, -t*PX,
			r*PX, -b*PX,
			color
		));
};

inline float abs_f (float x) { return x>0 ? x : -x; }
inline int sign_f (float x) { return (x>0)-(x<0); }



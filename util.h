

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define SWAP(a, b) {typeof(a) _t = b; b = a; a = _t;}

#define coords2sf(x, y) x, y

#define ID(x) (x)


#define UNPX (1.0/PX)

// Quick math assuming no negative numbers.
//#define round(x) ((int)((x)+0.5))
//#define floor(x) ((int)(x))

sf::Sprite drawing_sprite;
void draw_image (img::Image* image, float x, float y, int sub=0, bool flip=false, bool cam=false, float scale=1.0) {
	//if (!cam)
	//if (x < camera.x - 10 - img->w
	// || y < camera.y - 7.5 - img->h
	// || x > camera.x + 10 + img->w
	// || y > camera.y + 7.5 + img->h) return;
	uint iw = image->sfi.GetWidth();
	uint ih = image->sfi.GetHeight();
	sub %= image->numsubs();
	
	sf::IntRect sr;
	if (image->w == 0 || image->h == 0)
		sr = sf::IntRect(0, ih, iw, 0);
	else
		sr = sf::IntRect(
			sub % (iw/image->w) * image->w,
			sub / (iw/image->w) * image->h + image->h,
			sub % (iw/image->w) * image->w + image->w,
			sub / (iw/image->w) * image->h
		);
	drawing_sprite.SetScale(PX*scale, PX*scale);
	drawing_sprite.SetImage(image->sfi);
	drawing_sprite.SetSubRect(sr);
	drawing_sprite.FlipX(flip);
	drawing_sprite.FlipY(true);
	uint h = image->h;
	uint w = image->w;
	if (h == 0) h = ih;
	if (w == 0) w = iw;
	drawing_sprite.SetCenter(flip?w-image->x:image->x, -image->y);
//	uint xpos = flip && image->w ? image->w - image->x : image->x;
	if (!cam)
		drawing_sprite.SetPosition((round(x*UNPX)-.5)*PX, (round(y*UNPX)-.5)*PX);
	else
		drawing_sprite.SetPosition(
			window_view.GetRect().Left + x,
			window_view.GetRect().Top + y
		);
	window->Draw(drawing_sprite);
};
void draw_rect (float l, float t, float r, float b, sf::Color color=sf::Color(31, 31, 31, 127), bool cam=false) {
	sf::FloatRect vr = window_view.GetRect();
	if (cam)
		window->Draw(sf::Shape::Rectangle(
			vr.Left + l, vr.Top + t,
			vr.Left + r, vr.Top + b,
			color
		));
	else
		window->Draw(sf::Shape::Rectangle(
			round(l*UNPX)*PX, round(t*UNPX)*PX,
			round(r*UNPX)*PX, round(b*UNPX)*PX,
			color
		));
};

inline float mod_f (float a, float b) {
	return a >= 0 ? a - b*(double)(int)(a/b)
	              : a - b*(double)(int)(a/b);
}
inline float abs_f (float x) { return x>0 ? x : -x; }
inline int sign_f (float x) { return (x>0)-(x<0); }



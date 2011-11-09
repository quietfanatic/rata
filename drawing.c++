

#ifdef HEADER
 // Just let me use hexadecimal colors, okay?
struct Color {
	uint32 x;
	Color () { }
	Color (uint32 x) :x(x) { }
	Color (uint32 r, uint32 g, uint32 b, uint32 a) :x(r<<24|g<<16|b<<8|a) { }
	Color (sf::Color c) :x(c.r<<24|c.g<<16|c.b<<8|c.a) { }
	operator sf::Color () const {
		return sf::Color(x>>24, x>>16, x>>8, x);
	}
	bool visible () const { return (uint8)x; }
};
void draw_image (int16 imagei, Vec p, int sub=0, bool flip=false, bool cam=false, float scale=1.0);
void draw_rect (float l, float t, float r, float b, Color color = 0x2f2f2f7f, bool cam = false);
#else


sf::Sprite drawing_sprite;
void draw_image (int16 imagei, Vec p, int sub, bool flip, bool cam, float scale) {
	if (imagei < 0) return;
	img::Image* image = &img::def[imagei];
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
	float xpx = p.x*UNPX - (flip ? w-image->x : image->x);
	float ypx = p.y*UNPX + image->y;
//	uint xpos = flip && image->w ? image->w - image->x : image->x;
	if (!cam)
		drawing_sprite.SetPosition(
			(round(xpx)-.5)*PX,
			(round(ypx)-.5)*PX
		);
	else
		drawing_sprite.SetPosition(
			window_view.GetRect().Left + xpx*PX,
			window_view.GetRect().Top + ypx*PX
		);
	window->Draw(drawing_sprite);
};
void draw_rect (float l, float t, float r, float b, Color color, bool cam) {
	if (cam)
		window->Draw(sf::Shape::Rectangle(
			viewleft() + l, viewbottom() + t,
			viewleft() + r, viewbottom() + b,
			color
		));
	else
		window->Draw(sf::Shape::Rectangle(
			round(l*UNPX)*PX, round(t*UNPX)*PX,
			round(r*UNPX)*PX, round(b*UNPX)*PX,
			color
		));
};






#endif

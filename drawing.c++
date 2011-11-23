

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
	void setGL () const { glColor4ub(x>>24, x>>16, x>>8, x); }
};
void draw_image (img::Def* image, Vec p, int sub=0, bool flip=false, bool cam=false, float scale=1.0);
void draw_rect (float l, float t, float r, float b, Color color = 0x2f2f2f7f, bool cam = false);
void draw_line (Vec a, Vec v, Color color = 0xffffff7f, bool cam = false);
#else


sf::Sprite drawing_sprite;
void draw_image (img::Def* image, Vec p, int sub, bool flip, bool cam, float scale) {
	if (!image) return;
	sub %= image->numsubs();
	if (sub < 0) sub += image->numsubs();
	uint tw = image->sfi.GetWidth();
	uint th = image->sfi.GetHeight();
	uint iw = image->w ? image->w : image->sfi.GetWidth();
	uint ih = image->h ? image->h : image->sfi.GetHeight();
	uint subx = (sub % (tw / iw));
	uint suby = (sub / (tw / iw));
	float x = p.x - (flip ? iw-image->x : image->x)*PX;
	float y = p.y - (ih-image->y)*PX;
	if (!cam) {
		x -= camera.x - 10;
		y -= camera.y - 7.5;
	}

	float tl = subx*iw + iw*flip;
	float tr = subx*iw + iw*!flip;
	float tt = suby*ih;
	float tb = suby*ih + ih;
	
	glEnable(GL_TEXTURE_2D);
	image->sfi.Bind();
	glColor4f(1, 1, 1, 1);
	glBegin(GL_QUADS);
		glTexCoord2f(tl/tw, tb/th); glVertex2f(x,       y);
		glTexCoord2f(tr/tw, tb/th); glVertex2f(x+iw*PX, y);
		glTexCoord2f(tr/tw, tt/th); glVertex2f(x+iw*PX, y+ih*PX);
		glTexCoord2f(tl/tw, tt/th); glVertex2f(x,       y+ih*PX);
	glEnd();
	//window->Draw(drawing_sprite);
}
void draw_rect (float l, float t, float r, float b, Color color, bool cam) {
	glDisable(GL_TEXTURE_2D);
	color.setGL();
	if (!cam) {
		l -= camera.x - 10;
		t -= camera.y - 7.5;
		r -= camera.x - 10;
		b -= camera.y - 7.5;
	}
	glRectf(l, b, r, t);
};
void draw_line (Vec a, Vec b, Color color, bool cam) {
	glDisable(GL_TEXTURE_2D);
	color.setGL();
	glBegin(GL_LINES);
	if (!cam) {
		a -= Vec(camera.x-10, camera.y-7.5);
		b -= Vec(camera.x-10, camera.y-7.5);
	}
	glVertex2f(a.x, a.y);
	glVertex2f(b.x, b.y);
	glEnd();
};





#endif

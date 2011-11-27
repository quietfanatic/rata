

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
void vertex (Vec v);
void draw_image (img::Def* image, Vec p, int sub=0, bool flip=false);
void draw_rect (float l, float t, float r, float b, Color color = 0x2f2f2f7f);
void draw_line (Vec a, Vec b, Color color = 0xffffff7f);
#else
void vertex (Vec v) {
	glVertex2f(
		round(v.x*UNPX)*PX,
		round(v.y*UNPX)*PX
	);
}


sf::Sprite drawing_sprite;
void draw_image (img::Def* image, Vec p, int sub, bool flip) {
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

	float tl = subx*iw + iw*flip;
	float tr = subx*iw + iw*!flip;
	float tt = suby*ih;
	float tb = suby*ih + ih;
	
	glEnable(GL_TEXTURE_2D);
	image->sfi.Bind();
	glColor4f(1, 1, 1, 1);
	glBegin(GL_QUADS);
		glTexCoord2f(tl/tw, tb/th); vertex(Vec(x,       y      ));
		glTexCoord2f(tr/tw, tb/th); vertex(Vec(x+iw*PX, y      ));
		glTexCoord2f(tr/tw, tt/th); vertex(Vec(x+iw*PX, y+ih*PX));
		glTexCoord2f(tl/tw, tt/th); vertex(Vec(x,       y+ih*PX));
	glEnd();
	//window->Draw(drawing_sprite);
}
void draw_rect (float l, float t, float r, float b, Color color) {
	glDisable(GL_TEXTURE_2D);
	color.setGL();
	glRectf(l, b, r, t);
};
void draw_line (Vec a, Vec b, Color color) {
	glDisable(GL_TEXTURE_2D);
	color.setGL();
	glBegin(GL_LINES);
		vertex(a);
		vertex(b);
	glEnd();
};





#endif

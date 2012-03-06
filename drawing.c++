

#ifdef HEADER
 // Just let me use hexadecimal colors, okay?
struct Color {
	uint32 x;
	Color () { }
	Color (uint32 x) :x(x) { }
	Color (uint32 r, uint32 g, uint32 b, uint32 a) :x(r<<24|g<<16|b<<8|a) { }
	bool visible () const { return (uint8)x; }
	void setGL () const { glColor4ub(x>>24, x>>16, x>>8, x); }
};
void vertex (Vec v);
void draw_image (img::Def* image, Vec p, int sub=0, bool fliph=false, bool flipv=false);
void draw_rect (const Rect& r, Color color = 0x2f2f2f7f);
void draw_line (Vec a, Vec b, Color color = 0xffffff7f);
#else
void vertex (Vec v) {
	glVertex2f(
		round(v.x*UNPX)*PX,
		round(v.y*UNPX)*PX
	);
}


void draw_image (img::Def* image, Vec p, int sub, bool fliph, bool flipv) {
	if (!image) return;
	sub %= image->numsubs();
	if (sub < 0) sub += image->numsubs();
	uint tw = image->tw;
	uint th = image->th;
	uint iw = image->w;
	uint ih = image->h;
	uint subx = (sub % (tw / iw));
	uint suby = (sub / (tw / iw));
	float x = p.x - (fliph ? iw-image->x : image->x)*PX;
	float y = p.y - (flipv ? image->y : ih-image->y)*PX;

	float tl = (subx*iw + iw*fliph);///(float)tw;
	float tr = (subx*iw + iw*!fliph);///(float)tw;
	float tt = (suby*ih + ih*flipv);///(float)th;
	float tb = (suby*ih + ih*!flipv);///(float)th;
	
	glEnable(GL_TEXTURE_RECTANGLE);
	glBindTexture(GL_TEXTURE_RECTANGLE, image->tex);
	glColor4f(1, 1, 1, 1);
	glBegin(GL_QUADS);
		glTexCoord2f(tl, tb); vertex(Vec(x,       y      ));
		glTexCoord2f(tr, tb); vertex(Vec(x+iw*PX, y      ));
		glTexCoord2f(tr, tt); vertex(Vec(x+iw*PX, y+ih*PX));
		glTexCoord2f(tl, tt); vertex(Vec(x,       y+ih*PX));
	glEnd();
	dbg_draw("Drawing image at <%f v%f >%f ^%f\n", x, y, x+iw*PX, y+ih*PX);
}
void draw_rect (const Rect& r, Color color) {
	glDisable(GL_TEXTURE_RECTANGLE);
	color.setGL();
	glRectf(
		round(r.l*UNPX)*PX,
		round(r.b*UNPX)*PX,
		round(r.r*UNPX)*PX,
		round(r.t*UNPX)*PX
	);
	//dbg_draw("Drawing rect at <%f v%f >%f ^%f\n", r.l, r.b, r.r, r.t);
};
void draw_line (Vec a, Vec b, Color color) {
	glDisable(GL_TEXTURE_RECTANGLE);
	color.setGL();
	glBegin(GL_LINES);
		vertex(a);
		vertex(b);
	glEnd();
};

Vec circle_points [32] = {
	Vec(cos(pi*0/16.0), sin(pi*0/16.0)),
	Vec(cos(pi*1/16.0), sin(pi*1/16.0)),
	Vec(cos(pi*2/16.0), sin(pi*2/16.0)),
	Vec(cos(pi*3/16.0), sin(pi*3/16.0)),
	Vec(cos(pi*4/16.0), sin(pi*4/16.0)),
	Vec(cos(pi*5/16.0), sin(pi*5/16.0)),
	Vec(cos(pi*6/16.0), sin(pi*6/16.0)),
	Vec(cos(pi*7/16.0), sin(pi*7/16.0)),
	Vec(cos(pi*8/16.0), sin(pi*8/16.0)),
	Vec(cos(pi*9/16.0), sin(pi*9/16.0)),
	Vec(cos(pi*10/16.0), sin(pi*10/16.0)),
	Vec(cos(pi*11/16.0), sin(pi*11/16.0)),
	Vec(cos(pi*12/16.0), sin(pi*12/16.0)),
	Vec(cos(pi*13/16.0), sin(pi*13/16.0)),
	Vec(cos(pi*14/16.0), sin(pi*14/16.0)),
	Vec(cos(pi*15/16.0), sin(pi*15/16.0)),
	Vec(cos(pi*16/16.0), sin(pi*16/16.0)),
	Vec(cos(pi*17/16.0), sin(pi*17/16.0)),
	Vec(cos(pi*18/16.0), sin(pi*18/16.0)),
	Vec(cos(pi*19/16.0), sin(pi*19/16.0)),
	Vec(cos(pi*20/16.0), sin(pi*20/16.0)),
	Vec(cos(pi*21/16.0), sin(pi*21/16.0)),
	Vec(cos(pi*22/16.0), sin(pi*22/16.0)),
	Vec(cos(pi*23/16.0), sin(pi*23/16.0)),
	Vec(cos(pi*24/16.0), sin(pi*24/16.0)),
	Vec(cos(pi*25/16.0), sin(pi*25/16.0)),
	Vec(cos(pi*26/16.0), sin(pi*26/16.0)),
	Vec(cos(pi*27/16.0), sin(pi*27/16.0)),
	Vec(cos(pi*28/16.0), sin(pi*28/16.0)),
	Vec(cos(pi*29/16.0), sin(pi*29/16.0)),
	Vec(cos(pi*30/16.0), sin(pi*30/16.0)),
	Vec(cos(pi*31/16.0), sin(pi*31/16.0)),
};


void draw_circle (Vec p, float r, Color color) {
	color.setGL();
	glBegin(GL_LINE_LOOP);
	for (uint i=0; i < 32; i++) {
		vertex(p + r * circle_points[i]);
	}
	glEnd();
};




#endif

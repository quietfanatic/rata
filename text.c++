


const uint8 letter_width [256] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	3, 2, 6, 6, 6, 6, 6, 3, 4, 4, 6, 6, 3, 4, 3, 6,
	6, 4, 6, 6, 6, 6, 6, 6, 6, 6, 3, 3, 6, 6, 6, 6,
	8, 6, 6, 7, 7, 6, 6, 7, 6, 4, 6, 7, 6, 8, 8, 8,
	6, 8, 6, 6, 6, 8, 8, 8, 8, 8, 6, 4, 6, 4, 6, 6,
	3, 6, 6, 6, 6, 6, 5, 6, 6, 3, 3, 5, 3, 8, 6, 6,
	6, 6, 5, 6, 4, 6, 6, 6, 6, 6, 6, 5, 2, 5, 8, 0,
	6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const uint8 letter_width_small [256] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 4, 4, 4, 4, 4, 4, 5, 4, 4, 4, 4, 4, 6, 5, 4,
	4, 5, 4, 4, 4, 5, 4, 6, 4, 4, 4, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

uint text_width (char* s) {
	uint r;
	for (r = 0; *s; s++) {
		if (*s == '\n') break;
		r += letter_width[(uint8)*s];
	}
	return r;
};

uint text_width_small (char* s) {
	uint r;
	for (r = 0; *s; s++) {
		if (*s == '\n') break;
		r += letter_width_small[(uint8)*s];
	}
	return r;
};



void render_text (char* text, Vec p, uint lines, bool small, bool border, int align) {
	float width;
	uint linecount = 0;
	float lineheight = small ? 7*PX : 16*PX;
	if (border || align <= 0)
		width = (small ? text_width_small(text) : text_width(text))*PX;
	float lmargin = small ? 2*PX : 4*PX;
	float rmargin = small ? 1*PX : 4*PX;
	float start =
	  align > 0 ? p.x + lmargin
	: align < 0 ? p.x - width - rmargin
	:             p.x - width/2;
	if (border) draw::rect(Rect(start - lmargin, p.y, start + width + rmargin, p.y - lines*lineheight), 0x2f2f2f7f);

	float pos = start;
	img::Def* font = small ? img::font_small : img::font_proportional;
	for (;*text != 0 && linecount <= lines; text++) {
		//printf("%c;%d;%d;", *text, *text%16, *text/16);
		if (*text == '\n') {
			linecount++;
			pos = start+lmargin;
		}
		else {
			draw::image(font,
				Vec(pos, p.y-linecount*lineheight),
				*text, false
			);
			if (small)
				pos += letter_width_small[(uint8)*text]*PX;
			else pos += letter_width[(uint8)*text]*PX;
		}
	}
}






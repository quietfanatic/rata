


uint letter_width [256] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 6, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	3, 2, 6, 6, 6, 6, 6, 3, 4, 4, 6, 6, 3, 4, 3, 6,
	6, 4, 6, 6, 6, 6, 6, 6, 6, 6, 3, 3, 6, 6, 6, 6,
	8, 6, 6, 7, 7, 6, 6, 7, 6, 4, 6, 7, 6, 8, 8, 8,
	6, 8, 6, 6, 6, 8, 8, 8, 8, 8, 6, 4, 6, 4, 6, 6,
	3, 6, 6, 6, 6, 6, 5, 6, 6, 3, 3, 5, 3, 8, 6, 6,
	6, 6, 5, 6, 4, 6, 6, 6, 6, 6, 6, 5, 2, 5, 8, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


uint text_width (char* s) {
	uint r;
	for (r = 0; *s; s++) {
		r += letter_width[(uint8)*s];
		if (*s == '\n') break;
	}
	return r;
};




void render_text (char* text, float x, float y, uint lines=2, bool cam=false) {
	uint charcount = 0;
	uint linecount = 0;
	for (;*text != 0 && linecount <= lines; text++) {
		//printf("%c;%d;%d;", *text, *text%16, *text/16);
		if (*text == '\n') {
			linecount++;
			charcount = 0;
		}
		else {
			draw_image(&img::font_proportional,
				x + charcount*6*PX, y - linecount*8*PX,
				*text, false, cam
			);
			charcount++;
		}
	}
	//printdesc->x, desc->y
}







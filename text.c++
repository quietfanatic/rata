


//int letter_width



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
			draw_image_sub(img::font,
				x + charcount*6*PX, y - linecount*8*PX,
				*text % 16 * 6, *text / 16 * 8,
				*text % 16 * 6 + 6, *text / 16 * 8 + 8,
				false, cam
			);
			charcount++;
		}
	}
	//printdesc->x, desc->y
}







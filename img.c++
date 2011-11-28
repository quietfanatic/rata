
namespace img {
	struct Def {
		GLuint tex;
		const char* file;
		uint tw;
		uint th;
		uint w;
		uint h;
		float x;
		float y;
		uint numsubs () {
			if (w == 0 && h == 0) return 1;
			else return (tw / w) * (th / h);
		}
	};

	Def def [] = {
		{0, "img/0-metal;0,0.png", 128, 128, 0, 0, 0, 0},
		{0, "img/boots;32x32,16,30.png", 128, 96, 32, 32, 16, 30},
		{0, "img/crate;8,16.png", 16, 16, 0, 0, 8, 16},
		{0, "img/dress-arm;32x32,15,16.png", 96, 96, 32, 32, 15, 16},
		{0, "img/dress-body;32x32,16,30.png", 128, 96, 32, 32, 16, 30},
		{0, "img/dress-forearm;32x32,15,19.png", 96, 96, 32, 32, 15, 19},
		{0, "img/flyer;16x16,8,8.png", 144, 32, 16, 16, 8, 8},
		{0, "img/font-proportional;8x16,0,0.png", 128, 256, 8, 16, 0, 0},
		{0, "img/font-small;8x8,0,0.png", 128, 128, 8, 8, 0, 0},
		{0, "img/handgun;16x16,8,8.png", 80, 64, 16, 16, 8, 8},
		{0, "img/heart;8x8,4,8.png", 8, 40, 8, 8, 4, 8},
		{0, "img/helmet;17x17,8.5,8.5.png", 51, 34, 17, 17, 8.5, 8.5},
		{0, "img/hit-damagable;16x16,8,8.png", 16, 64, 16, 16, 8, 8},
		{0, "img/look;4.5,4.5.png", 9, 9, 0, 0, 4.5, 4.5},
		{0, "img/mousehole;8,16.png", 16, 16, 0, 0, 8, 16},
		{0, "img/nolook;4.5,4.5.png", 9, 9, 0, 0, 4.5, 4.5},
		{0, "img/patroller;16x16,8,16.png", 16, 32, 16, 16, 8, 16},
		{0, "img/rat;16x8,6,8.png", 16, 16, 16, 8, 6, 8},
		{0, "img/rata-arm;32x32,15,16.png", 96, 96, 32, 32, 15, 16},
		{0, "img/rata-body;32x32,16,30.png", 128, 96, 32, 32, 16, 30},
		{0, "img/rata-forearm;32x32,15,19.png", 96, 96, 32, 32, 15, 19},
		{0, "img/rata-head;32x32,16,14.png", 160, 128, 32, 32, 16, 14},
		{0, "img/readmore;4.5,4.5.png", 9, 9, 0, 0, 4.5, 4.5},
		{0, "img/see;4.5,4.5.png", 9, 9, 0, 0, 4.5, 4.5},
		{0, "img/target;4.5,4.5.png", 9, 9, 0, 0, 4.5, 4.5},
		{0, "img/tiles;16x16,8,8.png", 256, 512, 16, 16, 8, 8}
	};
	img::Def*const metal = def+0;
	img::Def*const boots = def+1;
	img::Def*const crate = def+2;
	img::Def*const dress_arm = def+3;
	img::Def*const dress_body = def+4;
	img::Def*const dress_forearm = def+5;
	img::Def*const flyer = def+6;
	img::Def*const font_proportional = def+7;
	img::Def*const font_small = def+8;
	img::Def*const handgun = def+9;
	img::Def*const heart = def+10;
	img::Def*const helmet = def+11;
	img::Def*const hit_damagable = def+12;
	img::Def*const look = def+13;
	img::Def*const mousehole = def+14;
	img::Def*const nolook = def+15;
	img::Def*const patroller = def+16;
	img::Def*const rat = def+17;
	img::Def*const rata_arm = def+18;
	img::Def*const rata_body = def+19;
	img::Def*const rata_forearm = def+20;
	img::Def*const rata_head = def+21;
	img::Def*const readmore = def+22;
	img::Def*const see = def+23;
	img::Def*const target = def+24;
	img::Def*const tiles = def+25;
	const uint n_imgs = 26;

}
void load_img () {
	for (uint i=0; i < img::n_imgs; i++) {
		img::def[i].tex = SOIL_load_OGL_texture(img::def[i].file, 4, 0, 0);
		if (!img::def[i].tex)
			printf("Error: Failed to load image %s.\n", img::def[i].file);
		glBindTexture(GL_TEXTURE_2D, img::def[i].tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
}



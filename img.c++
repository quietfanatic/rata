
namespace img {
	struct Def {
		sf::Image sfi;
		const char* file;
		uint w;
		uint h;
		float x;
		float y;
		uint numsubs () {
			if (w == 0 && h == 0) return 1;
			else return (sfi.GetWidth() / w) * (sfi.GetHeight() / h);
		}
	};

	Def def [] = {
		{sf::Image(), "img/0-metal;0,0.png", 0, 0, 0, 0},
		{sf::Image(), "img/boots;32x32,16,30.png", 32, 32, 16, 30},
		{sf::Image(), "img/crate;8,16.png", 0, 0, 8, 16},
		{sf::Image(), "img/dress-arm;32x32,15,16.png", 32, 32, 15, 16},
		{sf::Image(), "img/dress-body;32x32,16,30.png", 32, 32, 16, 30},
		{sf::Image(), "img/dress-forearm;32x32,15,19.png", 32, 32, 15, 19},
		{sf::Image(), "img/flyer;16x16,8,8.png", 16, 16, 8, 8},
		{sf::Image(), "img/font-proportional;8x16,0,0.png", 8, 16, 0, 0},
		{sf::Image(), "img/font-small;8x8,0,0.png", 8, 8, 0, 0},
		{sf::Image(), "img/handgun;16x16,8,8.png", 16, 16, 8, 8},
		{sf::Image(), "img/heart;8x8,4,8.png", 8, 8, 4, 8},
		{sf::Image(), "img/helmet;17x17,8.5,8.5.png", 17, 17, 8.5, 8.5},
		{sf::Image(), "img/hit-damagable;16x16,8,8.png", 16, 16, 8, 8},
		{sf::Image(), "img/look;4.5,4.5.png", 0, 0, 4.5, 4.5},
		{sf::Image(), "img/mousehole;8,16.png", 0, 0, 8, 16},
		{sf::Image(), "img/nolook;4.5,4.5.png", 0, 0, 4.5, 4.5},
		{sf::Image(), "img/patroller;16x16,8,16.png", 16, 16, 8, 16},
		{sf::Image(), "img/rat;16x8,6,8.png", 16, 8, 6, 8},
		{sf::Image(), "img/rata-arm;32x32,15,16.png", 32, 32, 15, 16},
		{sf::Image(), "img/rata-body;32x32,16,30.png", 32, 32, 16, 30},
		{sf::Image(), "img/rata-forearm;32x32,15,19.png", 32, 32, 15, 19},
		{sf::Image(), "img/rata-head;32x32,16,14.png", 32, 32, 16, 14},
		{sf::Image(), "img/readmore;4.5,4.5.png", 0, 0, 4.5, 4.5},
		{sf::Image(), "img/see;4.5,4.5.png", 0, 0, 4.5, 4.5},
		{sf::Image(), "img/target;4.5,4.5.png", 0, 0, 4.5, 4.5},
		{sf::Image(), "img/tiles;16x16,8,8.png", 16, 16, 8, 8}
	};
	static img::Def*const metal = def+0;
	static img::Def*const boots = def+1;
	static img::Def*const crate = def+2;
	static img::Def*const dress_arm = def+3;
	static img::Def*const dress_body = def+4;
	static img::Def*const dress_forearm = def+5;
	static img::Def*const flyer = def+6;
	static img::Def*const font_proportional = def+7;
	static img::Def*const font_small = def+8;
	static img::Def*const handgun = def+9;
	static img::Def*const heart = def+10;
	static img::Def*const helmet = def+11;
	static img::Def*const hit_damagable = def+12;
	static img::Def*const look = def+13;
	static img::Def*const mousehole = def+14;
	static img::Def*const nolook = def+15;
	static img::Def*const patroller = def+16;
	static img::Def*const rat = def+17;
	static img::Def*const rata_arm = def+18;
	static img::Def*const rata_body = def+19;
	static img::Def*const rata_forearm = def+20;
	static img::Def*const rata_head = def+21;
	static img::Def*const readmore = def+22;
	static img::Def*const see = def+23;
	static img::Def*const target = def+24;
	static img::Def*const tiles = def+25;
	static const uint n_imgs = 26;

}
void load_img () {
	for (uint i=0; i < img::n_imgs; i++) {
		if (!img::def[i].sfi.LoadFromFile(img::def[i].file))
			printf("Error: Failed to load image %s.\n", img::def[i].file);
		img::def[i].sfi.SetSmooth(0);
	}
}



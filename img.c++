
namespace img {
	struct Image {
		sf::Image sfi;
		uint w;
		uint h;
		float x;
		float y;
		uint numsubs () {
			if (w == 0 && h == 0) return 1;
			else return (sfi.GetWidth() / w) * (sfi.GetHeight() / h);
		}
	}
	bg0_metal,
	bg1_darklab,
	boots_kneel,
	boots_sit,
	boots_stand,
	boots_walk,
	boots,
	crate,
	dress_arm,
	dress_body,
	dress_forearm,
	font_6x16,
	font_8x16,
	font_proportional,
	font_small,
	font,
	handgun_hurtback,
	handgun_kneel,
	handgun_layback,
	handgun_m113,
	handgun_m135,
	handgun_m158,
	handgun_m23,
	handgun_m45,
	handgun_m68,
	handgun_m90,
	handgun_sit,
	handgun_stand,
	handgun_walk_b,
	handgun_walk_f,
	handgun,
	heart,
	helmet_135,
	helmet_158,
	helmet_23,
	helmet_45,
	helmet_90,
	helmet,
	hit_damagable,
	look,
	mousehole,
	nolook,
	patroller,
	rat,
	rata_arm,
	rata_body,
	rata_forearm,
	rata_head,
	readmore,
	see,
	target,
	testblock,
	testroom_fg,
	tiles,
	_COMMA_EATER;

	img::Image* _bgs [] = {
		&bg0_metal,
		&bg1_darklab,
		NULL
	};
}

void load_img () {
	bool good = true;
	good &= img::bg0_metal.sfi.LoadFromFile("img/bg0-metal;0,0.png"); img::bg0_metal.sfi.SetSmooth(0);
	img::bg0_metal.w = 0; img::bg0_metal.h = 0;
	img::bg0_metal.x = 0; img::bg0_metal.y = 0;
	good &= img::bg1_darklab.sfi.LoadFromFile("img/bg1-darklab;304x304,0,0.png"); img::bg1_darklab.sfi.SetSmooth(0);
	img::bg1_darklab.w = 304; img::bg1_darklab.h = 304;
	img::bg1_darklab.x = 0; img::bg1_darklab.y = 0;
	good &= img::boots_kneel.sfi.LoadFromFile("img/boots-kneel.png"); img::boots_kneel.sfi.SetSmooth(0);
	img::boots_kneel.w = 0; img::boots_kneel.h = 0;
	img::boots_kneel.x = 32; img::boots_kneel.y = 48;
	good &= img::boots_sit.sfi.LoadFromFile("img/boots-sit.png"); img::boots_sit.sfi.SetSmooth(0);
	img::boots_sit.w = 0; img::boots_sit.h = 0;
	img::boots_sit.x = 32; img::boots_sit.y = 48;
	good &= img::boots_stand.sfi.LoadFromFile("img/boots-stand.png"); img::boots_stand.sfi.SetSmooth(0);
	img::boots_stand.w = 0; img::boots_stand.h = 0;
	img::boots_stand.x = 32; img::boots_stand.y = 48;
	good &= img::boots_walk.sfi.LoadFromFile("img/boots-walk.png"); img::boots_walk.sfi.SetSmooth(0);
	img::boots_walk.w = 0; img::boots_walk.h = 0;
	img::boots_walk.x = 32; img::boots_walk.y = 48;
	good &= img::boots.sfi.LoadFromFile("img/boots;32x32,16,30.png"); img::boots.sfi.SetSmooth(0);
	img::boots.w = 32; img::boots.h = 32;
	img::boots.x = 16; img::boots.y = 30;
	good &= img::crate.sfi.LoadFromFile("img/crate;8,16.png"); img::crate.sfi.SetSmooth(0);
	img::crate.w = 0; img::crate.h = 0;
	img::crate.x = 8; img::crate.y = 16;
	good &= img::dress_arm.sfi.LoadFromFile("img/dress-arm;32x32,15,16.png"); img::dress_arm.sfi.SetSmooth(0);
	img::dress_arm.w = 32; img::dress_arm.h = 32;
	img::dress_arm.x = 15; img::dress_arm.y = 16;
	good &= img::dress_body.sfi.LoadFromFile("img/dress-body;32x32,16,30.png"); img::dress_body.sfi.SetSmooth(0);
	img::dress_body.w = 32; img::dress_body.h = 32;
	img::dress_body.x = 16; img::dress_body.y = 30;
	good &= img::dress_forearm.sfi.LoadFromFile("img/dress-forearm;32x32,15,19.png"); img::dress_forearm.sfi.SetSmooth(0);
	img::dress_forearm.w = 32; img::dress_forearm.h = 32;
	img::dress_forearm.x = 15; img::dress_forearm.y = 19;
	good &= img::font_6x16.sfi.LoadFromFile("img/font-6x16;0,0.png"); img::font_6x16.sfi.SetSmooth(0);
	img::font_6x16.w = 0; img::font_6x16.h = 0;
	img::font_6x16.x = 0; img::font_6x16.y = 0;
	good &= img::font_8x16.sfi.LoadFromFile("img/font-8x16;0,0.png"); img::font_8x16.sfi.SetSmooth(0);
	img::font_8x16.w = 0; img::font_8x16.h = 0;
	img::font_8x16.x = 0; img::font_8x16.y = 0;
	good &= img::font_proportional.sfi.LoadFromFile("img/font-proportional;8x16,0,0.png"); img::font_proportional.sfi.SetSmooth(0);
	img::font_proportional.w = 8; img::font_proportional.h = 16;
	img::font_proportional.x = 0; img::font_proportional.y = 0;
	good &= img::font_small.sfi.LoadFromFile("img/font-small;8x8,0,0.png"); img::font_small.sfi.SetSmooth(0);
	img::font_small.w = 8; img::font_small.h = 8;
	img::font_small.x = 0; img::font_small.y = 0;
	good &= img::font.sfi.LoadFromFile("img/font;0,0.png"); img::font.sfi.SetSmooth(0);
	img::font.w = 0; img::font.h = 0;
	img::font.x = 0; img::font.y = 0;
	good &= img::handgun_hurtback.sfi.LoadFromFile("img/handgun-hurtback.png"); img::handgun_hurtback.sfi.SetSmooth(0);
	img::handgun_hurtback.w = 0; img::handgun_hurtback.h = 0;
	img::handgun_hurtback.x = 32; img::handgun_hurtback.y = 48;
	good &= img::handgun_kneel.sfi.LoadFromFile("img/handgun-kneel.png"); img::handgun_kneel.sfi.SetSmooth(0);
	img::handgun_kneel.w = 0; img::handgun_kneel.h = 0;
	img::handgun_kneel.x = 32; img::handgun_kneel.y = 48;
	good &= img::handgun_layback.sfi.LoadFromFile("img/handgun-layback.png"); img::handgun_layback.sfi.SetSmooth(0);
	img::handgun_layback.w = 0; img::handgun_layback.h = 0;
	img::handgun_layback.x = 32; img::handgun_layback.y = 48;
	good &= img::handgun_m113.sfi.LoadFromFile("img/handgun-m113.png"); img::handgun_m113.sfi.SetSmooth(0);
	img::handgun_m113.w = 0; img::handgun_m113.h = 0;
	img::handgun_m113.x = 32; img::handgun_m113.y = 48;
	good &= img::handgun_m135.sfi.LoadFromFile("img/handgun-m135.png"); img::handgun_m135.sfi.SetSmooth(0);
	img::handgun_m135.w = 0; img::handgun_m135.h = 0;
	img::handgun_m135.x = 32; img::handgun_m135.y = 48;
	good &= img::handgun_m158.sfi.LoadFromFile("img/handgun-m158.png"); img::handgun_m158.sfi.SetSmooth(0);
	img::handgun_m158.w = 0; img::handgun_m158.h = 0;
	img::handgun_m158.x = 32; img::handgun_m158.y = 48;
	good &= img::handgun_m23.sfi.LoadFromFile("img/handgun-m23.png"); img::handgun_m23.sfi.SetSmooth(0);
	img::handgun_m23.w = 0; img::handgun_m23.h = 0;
	img::handgun_m23.x = 32; img::handgun_m23.y = 48;
	good &= img::handgun_m45.sfi.LoadFromFile("img/handgun-m45.png"); img::handgun_m45.sfi.SetSmooth(0);
	img::handgun_m45.w = 0; img::handgun_m45.h = 0;
	img::handgun_m45.x = 32; img::handgun_m45.y = 48;
	good &= img::handgun_m68.sfi.LoadFromFile("img/handgun-m68.png"); img::handgun_m68.sfi.SetSmooth(0);
	img::handgun_m68.w = 0; img::handgun_m68.h = 0;
	img::handgun_m68.x = 32; img::handgun_m68.y = 48;
	good &= img::handgun_m90.sfi.LoadFromFile("img/handgun-m90.png"); img::handgun_m90.sfi.SetSmooth(0);
	img::handgun_m90.w = 0; img::handgun_m90.h = 0;
	img::handgun_m90.x = 32; img::handgun_m90.y = 48;
	good &= img::handgun_sit.sfi.LoadFromFile("img/handgun-sit.png"); img::handgun_sit.sfi.SetSmooth(0);
	img::handgun_sit.w = 0; img::handgun_sit.h = 0;
	img::handgun_sit.x = 32; img::handgun_sit.y = 48;
	good &= img::handgun_stand.sfi.LoadFromFile("img/handgun-stand.png"); img::handgun_stand.sfi.SetSmooth(0);
	img::handgun_stand.w = 0; img::handgun_stand.h = 0;
	img::handgun_stand.x = 32; img::handgun_stand.y = 48;
	good &= img::handgun_walk_b.sfi.LoadFromFile("img/handgun-walk-b.png"); img::handgun_walk_b.sfi.SetSmooth(0);
	img::handgun_walk_b.w = 0; img::handgun_walk_b.h = 0;
	img::handgun_walk_b.x = 32; img::handgun_walk_b.y = 48;
	good &= img::handgun_walk_f.sfi.LoadFromFile("img/handgun-walk-f.png"); img::handgun_walk_f.sfi.SetSmooth(0);
	img::handgun_walk_f.w = 0; img::handgun_walk_f.h = 0;
	img::handgun_walk_f.x = 32; img::handgun_walk_f.y = 48;
	good &= img::handgun.sfi.LoadFromFile("img/handgun;16x16,8,8.png"); img::handgun.sfi.SetSmooth(0);
	img::handgun.w = 16; img::handgun.h = 16;
	img::handgun.x = 8; img::handgun.y = 8;
	good &= img::heart.sfi.LoadFromFile("img/heart;8x8,4,8.png"); img::heart.sfi.SetSmooth(0);
	img::heart.w = 8; img::heart.h = 8;
	img::heart.x = 4; img::heart.y = 8;
	good &= img::helmet_135.sfi.LoadFromFile("img/helmet-135.png"); img::helmet_135.sfi.SetSmooth(0);
	img::helmet_135.w = 0; img::helmet_135.h = 0;
	img::helmet_135.x = 32; img::helmet_135.y = 48;
	good &= img::helmet_158.sfi.LoadFromFile("img/helmet-158.png"); img::helmet_158.sfi.SetSmooth(0);
	img::helmet_158.w = 0; img::helmet_158.h = 0;
	img::helmet_158.x = 32; img::helmet_158.y = 48;
	good &= img::helmet_23.sfi.LoadFromFile("img/helmet-23.png"); img::helmet_23.sfi.SetSmooth(0);
	img::helmet_23.w = 0; img::helmet_23.h = 0;
	img::helmet_23.x = 32; img::helmet_23.y = 48;
	good &= img::helmet_45.sfi.LoadFromFile("img/helmet-45.png"); img::helmet_45.sfi.SetSmooth(0);
	img::helmet_45.w = 0; img::helmet_45.h = 0;
	img::helmet_45.x = 32; img::helmet_45.y = 48;
	good &= img::helmet_90.sfi.LoadFromFile("img/helmet-90.png"); img::helmet_90.sfi.SetSmooth(0);
	img::helmet_90.w = 0; img::helmet_90.h = 0;
	img::helmet_90.x = 32; img::helmet_90.y = 48;
	good &= img::helmet.sfi.LoadFromFile("img/helmet;17x17,8.5,8.5.png"); img::helmet.sfi.SetSmooth(0);
	img::helmet.w = 17; img::helmet.h = 17;
	img::helmet.x = 8.5; img::helmet.y = 8.5;
	good &= img::hit_damagable.sfi.LoadFromFile("img/hit-damagable;16x16,8,8.png"); img::hit_damagable.sfi.SetSmooth(0);
	img::hit_damagable.w = 16; img::hit_damagable.h = 16;
	img::hit_damagable.x = 8; img::hit_damagable.y = 8;
	good &= img::look.sfi.LoadFromFile("img/look;4.5,4.5.png"); img::look.sfi.SetSmooth(0);
	img::look.w = 0; img::look.h = 0;
	img::look.x = 4.5; img::look.y = 4.5;
	good &= img::mousehole.sfi.LoadFromFile("img/mousehole;8,16.png"); img::mousehole.sfi.SetSmooth(0);
	img::mousehole.w = 0; img::mousehole.h = 0;
	img::mousehole.x = 8; img::mousehole.y = 16;
	good &= img::nolook.sfi.LoadFromFile("img/nolook;4.5,4.5.png"); img::nolook.sfi.SetSmooth(0);
	img::nolook.w = 0; img::nolook.h = 0;
	img::nolook.x = 4.5; img::nolook.y = 4.5;
	good &= img::patroller.sfi.LoadFromFile("img/patroller;16x16,8,16.png"); img::patroller.sfi.SetSmooth(0);
	img::patroller.w = 16; img::patroller.h = 16;
	img::patroller.x = 8; img::patroller.y = 16;
	good &= img::rat.sfi.LoadFromFile("img/rat;16x8,6,8.png"); img::rat.sfi.SetSmooth(0);
	img::rat.w = 16; img::rat.h = 8;
	img::rat.x = 6; img::rat.y = 8;
	good &= img::rata_arm.sfi.LoadFromFile("img/rata-arm;32x32,15,16.png"); img::rata_arm.sfi.SetSmooth(0);
	img::rata_arm.w = 32; img::rata_arm.h = 32;
	img::rata_arm.x = 15; img::rata_arm.y = 16;
	good &= img::rata_body.sfi.LoadFromFile("img/rata-body;32x32,16,30.png"); img::rata_body.sfi.SetSmooth(0);
	img::rata_body.w = 32; img::rata_body.h = 32;
	img::rata_body.x = 16; img::rata_body.y = 30;
	good &= img::rata_forearm.sfi.LoadFromFile("img/rata-forearm;32x32,15,19.png"); img::rata_forearm.sfi.SetSmooth(0);
	img::rata_forearm.w = 32; img::rata_forearm.h = 32;
	img::rata_forearm.x = 15; img::rata_forearm.y = 19;
	good &= img::rata_head.sfi.LoadFromFile("img/rata-head;32x32,16,14.png"); img::rata_head.sfi.SetSmooth(0);
	img::rata_head.w = 32; img::rata_head.h = 32;
	img::rata_head.x = 16; img::rata_head.y = 14;
	good &= img::readmore.sfi.LoadFromFile("img/readmore;4.5,4.5.png"); img::readmore.sfi.SetSmooth(0);
	img::readmore.w = 0; img::readmore.h = 0;
	img::readmore.x = 4.5; img::readmore.y = 4.5;
	good &= img::see.sfi.LoadFromFile("img/see;4.5,4.5.png"); img::see.sfi.SetSmooth(0);
	img::see.w = 0; img::see.h = 0;
	img::see.x = 4.5; img::see.y = 4.5;
	good &= img::target.sfi.LoadFromFile("img/target;4.5,4.5.png"); img::target.sfi.SetSmooth(0);
	img::target.w = 0; img::target.h = 0;
	img::target.x = 4.5; img::target.y = 4.5;
	good &= img::testblock.sfi.LoadFromFile("img/testblock.png"); img::testblock.sfi.SetSmooth(0);
	img::testblock.w = 0; img::testblock.h = 0;
	img::testblock.x = 32; img::testblock.y = 48;
	good &= img::testroom_fg.sfi.LoadFromFile("img/testroom_fg.png"); img::testroom_fg.sfi.SetSmooth(0);
	img::testroom_fg.w = 0; img::testroom_fg.h = 0;
	img::testroom_fg.x = 32; img::testroom_fg.y = 48;
	good &= img::tiles.sfi.LoadFromFile("img/tiles;16x16,8,8.png"); img::tiles.sfi.SetSmooth(0);
	img::tiles.w = 16; img::tiles.h = 16;
	img::tiles.x = 8; img::tiles.y = 8;
	if (!good) fprintf(stderr, "Error: At least one image failed to load!\n");
}




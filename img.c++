
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
	boots,
	crate,
	dress_arm,
	dress_body,
	dress_forearm,
	flyer,
	font_proportional,
	font_small,
	handgun,
	heart,
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
	tiles;

	img::Image* _bgs [] = {
		&bg0_metal
	};

	void load_img () {
		bool good = true;
		good &= img::bg0_metal.sfi.LoadFromFile("img/bg0-metal;0,0.png"); img::bg0_metal.sfi.SetSmooth(0);
		img::bg0_metal.w = 0; img::bg0_metal.h = 0; img::bg0_metal.x = 0; img::bg0_metal.y = 0;
		good &= img::boots.sfi.LoadFromFile("img/boots;32x32,16,30.png"); img::boots.sfi.SetSmooth(0);
		img::boots.w = 32; img::boots.h = 32; img::boots.x = 16; img::boots.y = 30;
		good &= img::crate.sfi.LoadFromFile("img/crate;8,16.png"); img::crate.sfi.SetSmooth(0);
		img::crate.w = 0; img::crate.h = 0; img::crate.x = 8; img::crate.y = 16;
		good &= img::dress_arm.sfi.LoadFromFile("img/dress-arm;32x32,15,16.png"); img::dress_arm.sfi.SetSmooth(0);
		img::dress_arm.w = 32; img::dress_arm.h = 32; img::dress_arm.x = 15; img::dress_arm.y = 16;
		good &= img::dress_body.sfi.LoadFromFile("img/dress-body;32x32,16,30.png"); img::dress_body.sfi.SetSmooth(0);
		img::dress_body.w = 32; img::dress_body.h = 32; img::dress_body.x = 16; img::dress_body.y = 30;
		good &= img::dress_forearm.sfi.LoadFromFile("img/dress-forearm;32x32,15,19.png"); img::dress_forearm.sfi.SetSmooth(0);
		img::dress_forearm.w = 32; img::dress_forearm.h = 32; img::dress_forearm.x = 15; img::dress_forearm.y = 19;
		good &= img::flyer.sfi.LoadFromFile("img/flyer;16x16,8,8.png"); img::flyer.sfi.SetSmooth(0);
		img::flyer.w = 16; img::flyer.h = 16; img::flyer.x = 8; img::flyer.y = 8;
		good &= img::font_proportional.sfi.LoadFromFile("img/font-proportional;8x16,0,0.png"); img::font_proportional.sfi.SetSmooth(0);
		img::font_proportional.w = 8; img::font_proportional.h = 16; img::font_proportional.x = 0; img::font_proportional.y = 0;
		good &= img::font_small.sfi.LoadFromFile("img/font-small;8x8,0,0.png"); img::font_small.sfi.SetSmooth(0);
		img::font_small.w = 8; img::font_small.h = 8; img::font_small.x = 0; img::font_small.y = 0;
		good &= img::handgun.sfi.LoadFromFile("img/handgun;16x16,8,8.png"); img::handgun.sfi.SetSmooth(0);
		img::handgun.w = 16; img::handgun.h = 16; img::handgun.x = 8; img::handgun.y = 8;
		good &= img::heart.sfi.LoadFromFile("img/heart;8x8,4,8.png"); img::heart.sfi.SetSmooth(0);
		img::heart.w = 8; img::heart.h = 8; img::heart.x = 4; img::heart.y = 8;
		good &= img::helmet.sfi.LoadFromFile("img/helmet;17x17,8.5,8.5.png"); img::helmet.sfi.SetSmooth(0);
		img::helmet.w = 17; img::helmet.h = 17; img::helmet.x = 8.5; img::helmet.y = 8.5;
		good &= img::hit_damagable.sfi.LoadFromFile("img/hit-damagable;16x16,8,8.png"); img::hit_damagable.sfi.SetSmooth(0);
		img::hit_damagable.w = 16; img::hit_damagable.h = 16; img::hit_damagable.x = 8; img::hit_damagable.y = 8;
		good &= img::look.sfi.LoadFromFile("img/look;4.5,4.5.png"); img::look.sfi.SetSmooth(0);
		img::look.w = 0; img::look.h = 0; img::look.x = 4.5; img::look.y = 4.5;
		good &= img::mousehole.sfi.LoadFromFile("img/mousehole;8,16.png"); img::mousehole.sfi.SetSmooth(0);
		img::mousehole.w = 0; img::mousehole.h = 0; img::mousehole.x = 8; img::mousehole.y = 16;
		good &= img::nolook.sfi.LoadFromFile("img/nolook;4.5,4.5.png"); img::nolook.sfi.SetSmooth(0);
		img::nolook.w = 0; img::nolook.h = 0; img::nolook.x = 4.5; img::nolook.y = 4.5;
		good &= img::patroller.sfi.LoadFromFile("img/patroller;16x16,8,16.png"); img::patroller.sfi.SetSmooth(0);
		img::patroller.w = 16; img::patroller.h = 16; img::patroller.x = 8; img::patroller.y = 16;
		good &= img::rat.sfi.LoadFromFile("img/rat;16x8,6,8.png"); img::rat.sfi.SetSmooth(0);
		img::rat.w = 16; img::rat.h = 8; img::rat.x = 6; img::rat.y = 8;
		good &= img::rata_arm.sfi.LoadFromFile("img/rata-arm;32x32,15,16.png"); img::rata_arm.sfi.SetSmooth(0);
		img::rata_arm.w = 32; img::rata_arm.h = 32; img::rata_arm.x = 15; img::rata_arm.y = 16;
		good &= img::rata_body.sfi.LoadFromFile("img/rata-body;32x32,16,30.png"); img::rata_body.sfi.SetSmooth(0);
		img::rata_body.w = 32; img::rata_body.h = 32; img::rata_body.x = 16; img::rata_body.y = 30;
		good &= img::rata_forearm.sfi.LoadFromFile("img/rata-forearm;32x32,15,19.png"); img::rata_forearm.sfi.SetSmooth(0);
		img::rata_forearm.w = 32; img::rata_forearm.h = 32; img::rata_forearm.x = 15; img::rata_forearm.y = 19;
		good &= img::rata_head.sfi.LoadFromFile("img/rata-head;32x32,16,14.png"); img::rata_head.sfi.SetSmooth(0);
		img::rata_head.w = 32; img::rata_head.h = 32; img::rata_head.x = 16; img::rata_head.y = 14;
		good &= img::readmore.sfi.LoadFromFile("img/readmore;4.5,4.5.png"); img::readmore.sfi.SetSmooth(0);
		img::readmore.w = 0; img::readmore.h = 0; img::readmore.x = 4.5; img::readmore.y = 4.5;
		good &= img::see.sfi.LoadFromFile("img/see;4.5,4.5.png"); img::see.sfi.SetSmooth(0);
		img::see.w = 0; img::see.h = 0; img::see.x = 4.5; img::see.y = 4.5;
		good &= img::target.sfi.LoadFromFile("img/target;4.5,4.5.png"); img::target.sfi.SetSmooth(0);
		img::target.w = 0; img::target.h = 0; img::target.x = 4.5; img::target.y = 4.5;
		good &= img::tiles.sfi.LoadFromFile("img/tiles;16x16,8,8.png"); img::tiles.sfi.SetSmooth(0);
		img::tiles.w = 16; img::tiles.h = 16; img::tiles.x = 8; img::tiles.y = 8;
		if (!good) fprintf(stderr, "Error: At least one image failed to load!\n");
	}
}



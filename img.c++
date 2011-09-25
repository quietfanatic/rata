
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
	arm_dress_hurtback,
	arm_dress_kneel,
	arm_dress_layback,
	arm_dress_m113,
	arm_dress_m135,
	arm_dress_m158,
	arm_dress_m23,
	arm_dress_m45,
	arm_dress_m68,
	arm_dress_m90,
	arm_dress_push_low,
	arm_dress_push,
	arm_dress_sit,
	arm_dress_stand,
	arm_dress_standback,
	arm_dress_walk_b,
	arm_dress_walk_f,
	arm_m90,
	arm_stand,
	bg0_metal,
	bg1_darklab,
	body_dress_hurtback,
	body_dress_kneel,
	body_dress_layback,
	body_dress_leanforward,
	body_dress_sit,
	body_dress_stand,
	body_dress_tiptoe,
	body_dress_walk,
	body_hurtback,
	body_sit,
	body_stand_copy,
	body_stand,
	body_walk,
	boots_kneel,
	boots_sit,
	boots_stand,
	boots_walk,
	crate,
	font_6x16,
	font_8x16,
	font_proportional,
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
	head_hurtback,
	head_layback,
	head_stand_135,
	head_stand_158,
	head_stand_23,
	head_stand_45,
	head_stand_68,
	head_stand_90,
	head_walk_135,
	head_walk_158,
	head_walk_23,
	head_walk_45,
	head_walk_68,
	head_walk_90,
	heart,
	helmet_135,
	helmet_158,
	helmet_23,
	helmet_45,
	helmet_90,
	hit_damagable,
	look,
	mousehole,
	nolook,
	patroller,
	rat,
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
	good &= img::arm_dress_hurtback.sfi.LoadFromFile("img/arm+dress-hurtback.png"); img::arm_dress_hurtback.sfi.SetSmooth(0);
	img::arm_dress_hurtback.w = 0; img::arm_dress_hurtback.h = 0;
	img::arm_dress_hurtback.x = 32; img::arm_dress_hurtback.y = 48;
	good &= img::arm_dress_kneel.sfi.LoadFromFile("img/arm+dress-kneel.png"); img::arm_dress_kneel.sfi.SetSmooth(0);
	img::arm_dress_kneel.w = 0; img::arm_dress_kneel.h = 0;
	img::arm_dress_kneel.x = 32; img::arm_dress_kneel.y = 48;
	good &= img::arm_dress_layback.sfi.LoadFromFile("img/arm+dress-layback.png"); img::arm_dress_layback.sfi.SetSmooth(0);
	img::arm_dress_layback.w = 0; img::arm_dress_layback.h = 0;
	img::arm_dress_layback.x = 32; img::arm_dress_layback.y = 48;
	good &= img::arm_dress_m113.sfi.LoadFromFile("img/arm+dress-m113.png"); img::arm_dress_m113.sfi.SetSmooth(0);
	img::arm_dress_m113.w = 0; img::arm_dress_m113.h = 0;
	img::arm_dress_m113.x = 32; img::arm_dress_m113.y = 48;
	good &= img::arm_dress_m135.sfi.LoadFromFile("img/arm+dress-m135.png"); img::arm_dress_m135.sfi.SetSmooth(0);
	img::arm_dress_m135.w = 0; img::arm_dress_m135.h = 0;
	img::arm_dress_m135.x = 32; img::arm_dress_m135.y = 48;
	good &= img::arm_dress_m158.sfi.LoadFromFile("img/arm+dress-m158.png"); img::arm_dress_m158.sfi.SetSmooth(0);
	img::arm_dress_m158.w = 0; img::arm_dress_m158.h = 0;
	img::arm_dress_m158.x = 32; img::arm_dress_m158.y = 48;
	good &= img::arm_dress_m23.sfi.LoadFromFile("img/arm+dress-m23.png"); img::arm_dress_m23.sfi.SetSmooth(0);
	img::arm_dress_m23.w = 0; img::arm_dress_m23.h = 0;
	img::arm_dress_m23.x = 32; img::arm_dress_m23.y = 48;
	good &= img::arm_dress_m45.sfi.LoadFromFile("img/arm+dress-m45.png"); img::arm_dress_m45.sfi.SetSmooth(0);
	img::arm_dress_m45.w = 0; img::arm_dress_m45.h = 0;
	img::arm_dress_m45.x = 32; img::arm_dress_m45.y = 48;
	good &= img::arm_dress_m68.sfi.LoadFromFile("img/arm+dress-m68.png"); img::arm_dress_m68.sfi.SetSmooth(0);
	img::arm_dress_m68.w = 0; img::arm_dress_m68.h = 0;
	img::arm_dress_m68.x = 32; img::arm_dress_m68.y = 48;
	good &= img::arm_dress_m90.sfi.LoadFromFile("img/arm+dress-m90.png"); img::arm_dress_m90.sfi.SetSmooth(0);
	img::arm_dress_m90.w = 0; img::arm_dress_m90.h = 0;
	img::arm_dress_m90.x = 32; img::arm_dress_m90.y = 48;
	good &= img::arm_dress_push_low.sfi.LoadFromFile("img/arm+dress-push-low.png"); img::arm_dress_push_low.sfi.SetSmooth(0);
	img::arm_dress_push_low.w = 0; img::arm_dress_push_low.h = 0;
	img::arm_dress_push_low.x = 32; img::arm_dress_push_low.y = 48;
	good &= img::arm_dress_push.sfi.LoadFromFile("img/arm+dress-push.png"); img::arm_dress_push.sfi.SetSmooth(0);
	img::arm_dress_push.w = 0; img::arm_dress_push.h = 0;
	img::arm_dress_push.x = 32; img::arm_dress_push.y = 48;
	good &= img::arm_dress_sit.sfi.LoadFromFile("img/arm+dress-sit.png"); img::arm_dress_sit.sfi.SetSmooth(0);
	img::arm_dress_sit.w = 0; img::arm_dress_sit.h = 0;
	img::arm_dress_sit.x = 32; img::arm_dress_sit.y = 48;
	good &= img::arm_dress_stand.sfi.LoadFromFile("img/arm+dress-stand.png"); img::arm_dress_stand.sfi.SetSmooth(0);
	img::arm_dress_stand.w = 0; img::arm_dress_stand.h = 0;
	img::arm_dress_stand.x = 32; img::arm_dress_stand.y = 48;
	good &= img::arm_dress_standback.sfi.LoadFromFile("img/arm+dress-standback.png"); img::arm_dress_standback.sfi.SetSmooth(0);
	img::arm_dress_standback.w = 0; img::arm_dress_standback.h = 0;
	img::arm_dress_standback.x = 32; img::arm_dress_standback.y = 48;
	good &= img::arm_dress_walk_b.sfi.LoadFromFile("img/arm+dress-walk-b.png"); img::arm_dress_walk_b.sfi.SetSmooth(0);
	img::arm_dress_walk_b.w = 0; img::arm_dress_walk_b.h = 0;
	img::arm_dress_walk_b.x = 32; img::arm_dress_walk_b.y = 48;
	good &= img::arm_dress_walk_f.sfi.LoadFromFile("img/arm+dress-walk-f.png"); img::arm_dress_walk_f.sfi.SetSmooth(0);
	img::arm_dress_walk_f.w = 0; img::arm_dress_walk_f.h = 0;
	img::arm_dress_walk_f.x = 32; img::arm_dress_walk_f.y = 48;
	good &= img::arm_m90.sfi.LoadFromFile("img/arm-m90.png"); img::arm_m90.sfi.SetSmooth(0);
	img::arm_m90.w = 0; img::arm_m90.h = 0;
	img::arm_m90.x = 32; img::arm_m90.y = 48;
	good &= img::arm_stand.sfi.LoadFromFile("img/arm-stand.png"); img::arm_stand.sfi.SetSmooth(0);
	img::arm_stand.w = 0; img::arm_stand.h = 0;
	img::arm_stand.x = 32; img::arm_stand.y = 48;
	good &= img::bg0_metal.sfi.LoadFromFile("img/bg0-metal;0,0.png"); img::bg0_metal.sfi.SetSmooth(0);
	img::bg0_metal.w = 0; img::bg0_metal.h = 0;
	img::bg0_metal.x = 0; img::bg0_metal.y = 0;
	good &= img::bg1_darklab.sfi.LoadFromFile("img/bg1-darklab;38x38,0,0.png"); img::bg1_darklab.sfi.SetSmooth(0);
	img::bg1_darklab.w = 38; img::bg1_darklab.h = 38;
	img::bg1_darklab.x = 0; img::bg1_darklab.y = 0;
	good &= img::body_dress_hurtback.sfi.LoadFromFile("img/body+dress-hurtback.png"); img::body_dress_hurtback.sfi.SetSmooth(0);
	img::body_dress_hurtback.w = 0; img::body_dress_hurtback.h = 0;
	img::body_dress_hurtback.x = 32; img::body_dress_hurtback.y = 48;
	good &= img::body_dress_kneel.sfi.LoadFromFile("img/body+dress-kneel.png"); img::body_dress_kneel.sfi.SetSmooth(0);
	img::body_dress_kneel.w = 0; img::body_dress_kneel.h = 0;
	img::body_dress_kneel.x = 32; img::body_dress_kneel.y = 48;
	good &= img::body_dress_layback.sfi.LoadFromFile("img/body+dress-layback.png"); img::body_dress_layback.sfi.SetSmooth(0);
	img::body_dress_layback.w = 0; img::body_dress_layback.h = 0;
	img::body_dress_layback.x = 32; img::body_dress_layback.y = 48;
	good &= img::body_dress_leanforward.sfi.LoadFromFile("img/body+dress-leanforward.png"); img::body_dress_leanforward.sfi.SetSmooth(0);
	img::body_dress_leanforward.w = 0; img::body_dress_leanforward.h = 0;
	img::body_dress_leanforward.x = 32; img::body_dress_leanforward.y = 48;
	good &= img::body_dress_sit.sfi.LoadFromFile("img/body+dress-sit.png"); img::body_dress_sit.sfi.SetSmooth(0);
	img::body_dress_sit.w = 0; img::body_dress_sit.h = 0;
	img::body_dress_sit.x = 32; img::body_dress_sit.y = 48;
	good &= img::body_dress_stand.sfi.LoadFromFile("img/body+dress-stand.png"); img::body_dress_stand.sfi.SetSmooth(0);
	img::body_dress_stand.w = 0; img::body_dress_stand.h = 0;
	img::body_dress_stand.x = 32; img::body_dress_stand.y = 48;
	good &= img::body_dress_tiptoe.sfi.LoadFromFile("img/body+dress-tiptoe.png"); img::body_dress_tiptoe.sfi.SetSmooth(0);
	img::body_dress_tiptoe.w = 0; img::body_dress_tiptoe.h = 0;
	img::body_dress_tiptoe.x = 32; img::body_dress_tiptoe.y = 48;
	good &= img::body_dress_walk.sfi.LoadFromFile("img/body+dress-walk.png"); img::body_dress_walk.sfi.SetSmooth(0);
	img::body_dress_walk.w = 0; img::body_dress_walk.h = 0;
	img::body_dress_walk.x = 32; img::body_dress_walk.y = 48;
	good &= img::body_hurtback.sfi.LoadFromFile("img/body-hurtback.png"); img::body_hurtback.sfi.SetSmooth(0);
	img::body_hurtback.w = 0; img::body_hurtback.h = 0;
	img::body_hurtback.x = 32; img::body_hurtback.y = 48;
	good &= img::body_sit.sfi.LoadFromFile("img/body-sit.png"); img::body_sit.sfi.SetSmooth(0);
	img::body_sit.w = 0; img::body_sit.h = 0;
	img::body_sit.x = 32; img::body_sit.y = 48;
	good &= img::body_stand_copy.sfi.LoadFromFile("img/body-stand copy.png"); img::body_stand_copy.sfi.SetSmooth(0);
	img::body_stand_copy.w = 0; img::body_stand_copy.h = 0;
	img::body_stand_copy.x = 32; img::body_stand_copy.y = 48;
	good &= img::body_stand.sfi.LoadFromFile("img/body-stand.png"); img::body_stand.sfi.SetSmooth(0);
	img::body_stand.w = 0; img::body_stand.h = 0;
	img::body_stand.x = 32; img::body_stand.y = 48;
	good &= img::body_walk.sfi.LoadFromFile("img/body-walk.png"); img::body_walk.sfi.SetSmooth(0);
	img::body_walk.w = 0; img::body_walk.h = 0;
	img::body_walk.x = 32; img::body_walk.y = 48;
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
	good &= img::crate.sfi.LoadFromFile("img/crate;8,16.png"); img::crate.sfi.SetSmooth(0);
	img::crate.w = 0; img::crate.h = 0;
	img::crate.x = 8; img::crate.y = 16;
	good &= img::font_6x16.sfi.LoadFromFile("img/font-6x16;0,0.png"); img::font_6x16.sfi.SetSmooth(0);
	img::font_6x16.w = 0; img::font_6x16.h = 0;
	img::font_6x16.x = 0; img::font_6x16.y = 0;
	good &= img::font_8x16.sfi.LoadFromFile("img/font-8x16;0,0.png"); img::font_8x16.sfi.SetSmooth(0);
	img::font_8x16.w = 0; img::font_8x16.h = 0;
	img::font_8x16.x = 0; img::font_8x16.y = 0;
	good &= img::font_proportional.sfi.LoadFromFile("img/font-proportional;8x16,0,0.png"); img::font_proportional.sfi.SetSmooth(0);
	img::font_proportional.w = 8; img::font_proportional.h = 16;
	img::font_proportional.x = 0; img::font_proportional.y = 0;
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
	good &= img::head_hurtback.sfi.LoadFromFile("img/head-hurtback.png"); img::head_hurtback.sfi.SetSmooth(0);
	img::head_hurtback.w = 0; img::head_hurtback.h = 0;
	img::head_hurtback.x = 32; img::head_hurtback.y = 48;
	good &= img::head_layback.sfi.LoadFromFile("img/head-layback.png"); img::head_layback.sfi.SetSmooth(0);
	img::head_layback.w = 0; img::head_layback.h = 0;
	img::head_layback.x = 32; img::head_layback.y = 48;
	good &= img::head_stand_135.sfi.LoadFromFile("img/head-stand-135.png"); img::head_stand_135.sfi.SetSmooth(0);
	img::head_stand_135.w = 0; img::head_stand_135.h = 0;
	img::head_stand_135.x = 32; img::head_stand_135.y = 48;
	good &= img::head_stand_158.sfi.LoadFromFile("img/head-stand-158.png"); img::head_stand_158.sfi.SetSmooth(0);
	img::head_stand_158.w = 0; img::head_stand_158.h = 0;
	img::head_stand_158.x = 32; img::head_stand_158.y = 48;
	good &= img::head_stand_23.sfi.LoadFromFile("img/head-stand-23.png"); img::head_stand_23.sfi.SetSmooth(0);
	img::head_stand_23.w = 0; img::head_stand_23.h = 0;
	img::head_stand_23.x = 32; img::head_stand_23.y = 48;
	good &= img::head_stand_45.sfi.LoadFromFile("img/head-stand-45.png"); img::head_stand_45.sfi.SetSmooth(0);
	img::head_stand_45.w = 0; img::head_stand_45.h = 0;
	img::head_stand_45.x = 32; img::head_stand_45.y = 48;
	good &= img::head_stand_68.sfi.LoadFromFile("img/head-stand-68.png"); img::head_stand_68.sfi.SetSmooth(0);
	img::head_stand_68.w = 0; img::head_stand_68.h = 0;
	img::head_stand_68.x = 32; img::head_stand_68.y = 48;
	good &= img::head_stand_90.sfi.LoadFromFile("img/head-stand-90.png"); img::head_stand_90.sfi.SetSmooth(0);
	img::head_stand_90.w = 0; img::head_stand_90.h = 0;
	img::head_stand_90.x = 32; img::head_stand_90.y = 48;
	good &= img::head_walk_135.sfi.LoadFromFile("img/head-walk-135.png"); img::head_walk_135.sfi.SetSmooth(0);
	img::head_walk_135.w = 0; img::head_walk_135.h = 0;
	img::head_walk_135.x = 32; img::head_walk_135.y = 48;
	good &= img::head_walk_158.sfi.LoadFromFile("img/head-walk-158.png"); img::head_walk_158.sfi.SetSmooth(0);
	img::head_walk_158.w = 0; img::head_walk_158.h = 0;
	img::head_walk_158.x = 32; img::head_walk_158.y = 48;
	good &= img::head_walk_23.sfi.LoadFromFile("img/head-walk-23.png"); img::head_walk_23.sfi.SetSmooth(0);
	img::head_walk_23.w = 0; img::head_walk_23.h = 0;
	img::head_walk_23.x = 32; img::head_walk_23.y = 48;
	good &= img::head_walk_45.sfi.LoadFromFile("img/head-walk-45.png"); img::head_walk_45.sfi.SetSmooth(0);
	img::head_walk_45.w = 0; img::head_walk_45.h = 0;
	img::head_walk_45.x = 32; img::head_walk_45.y = 48;
	good &= img::head_walk_68.sfi.LoadFromFile("img/head-walk-68.png"); img::head_walk_68.sfi.SetSmooth(0);
	img::head_walk_68.w = 0; img::head_walk_68.h = 0;
	img::head_walk_68.x = 32; img::head_walk_68.y = 48;
	good &= img::head_walk_90.sfi.LoadFromFile("img/head-walk-90.png"); img::head_walk_90.sfi.SetSmooth(0);
	img::head_walk_90.w = 0; img::head_walk_90.h = 0;
	img::head_walk_90.x = 32; img::head_walk_90.y = 48;
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




#include <GL/glfw.h>
#include "../../hacc/inc/everything.h"
#include "../../core/inc/state.h"
#include "../../core/inc/input.h"
#include "../../ent/inc/humans.h"
#include "../../vis/inc/sprites.h"
#include "../../vis/inc/text.h"

using namespace phys;
using namespace ent;
using namespace vis;
using namespace core;
using namespace geo;

 // For now we're skipping having a generic Human class
static BodyDef*& bdf () {
     static auto bdf = hacc::reference_file<BodyDef>("modules/rata/res/rata.bdf");
     return bdf;
}
static Skel*& skel () {
    static auto skel = hacc::reference_file<Skel>("modules/rata/res/rata.skel");
    return skel;
}

static Biped::Stats*& stats () {
    static auto stats = hacc::reference_file<Biped::Stats>("modules/rata/res/rata.stats");
    return stats;
}

static Skin*& base () {
    static auto base = hacc::reference_file<Skin>("modules/rata/res/rata-base.skin");
    return base;
}

static Font*& monospace () {
    static auto monospace = hacc::reference_file<Font>("modules/rata/res/monospace.font");
    return monospace;
}

struct Rata : Stateful, Biped, Draws_Text, Key_Listener {
    Controls controls;

    bool hear_key (int keycode, int action) {
        bool on = action == GLFW_PRESS;
        switch (keycode) {
            case 'A': controls.left = on; return true;
            case 'D': controls.right = on; return true;
            case 'W': controls.jump = on; return true;
            case 'S': controls.crouch = on; return true;
            default: return false;
        }
    }

    void draws_sprites () {
        model.apply_skin(base());
        Biped::draws_sprites();
    }
    void text_draw () {
        draw_text("asdf", monospace(), Vec(4, 4));
    }

    void before_move () {
        allow_movement(stats(), &controls);
    }

    Rata () : Biped(bdf(), skel()) { }
    void emerge () { printf("Emerging\n"); Biped::emerge(); text_appear(); }
    void reclude () { Biped::reclude(); text_disappear(); }
    void start () { geography->behold(this); }
};

HCB_BEGIN(Rata)
    type_name("Rata");
    base<Stateful>("Rata");
    attr("biped", supertype<Biped>());
HCB_END(Rata)


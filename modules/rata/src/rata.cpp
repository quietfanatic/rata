#include "GL/glfw.h"
#include "../../hacc/inc/everything.h"
#include "../../core/inc/state.h"
#include "../../core/inc/input.h"
#include "../../ent/inc/humans.h"
#include "../../vis/inc/sprites.h"

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

struct Rata : Stateful, Biped {

    void draw () {
        model.apply_skin(base());
        Biped::draw();
    }

    void before_move () {
        Controls controls;
        controls.left = get_key('A');
        controls.right = get_key('D');
        controls.jump = get_key('W');
        controls.crouch = get_key('S');
        allow_movement(stats(), &controls);
    }

    Rata () : Biped(bdf(), skel()) { }
    void emerge () { printf("Emerging\n"); Biped::emerge(); }
    void reclude () { }
    void start () { beholder = this; }
};

HCB_BEGIN(Rata)
    type_name("Rata");
    base<Stateful>("Rata");
    attr("biped", supertype<Biped>());
HCB_END(Rata)



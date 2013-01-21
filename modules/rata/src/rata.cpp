
#include "../../hacc/inc/everything.h"
#include "../../core/inc/state.h"
#include "../../core/inc/input.h"
#include "../../phys/inc/phys.h"
#include "../../vis/inc/sprites.h"
#include "../../vis/inc/models.h"

using phys::Physical;
using namespace vis;

 // For now we're skipping having a generic Human class
static phys::BodyDef*& bdf () {
     static auto bdf = hacc::reference_file<phys::BodyDef>("modules/rata/res/rata.bdf");
     return bdf;
}
static vis::Skel*& skel () {
    static auto skel = hacc::reference_file<vis::Skel>("modules/rata/res/rata.skel");
    return skel;
}

struct Rata : core::Stateful, phys::Physical, vis::Draws_Sprites {
     // For drawing
    Model model;
    void draw () {
        static auto stand = skel()->poses.named("stand");
        static auto base = hacc::reference_file<vis::Skin>("modules/rata/res/rata-base.skin");
        model.apply_pose(stand);
        model.apply_skin(base);
        model.draw(pos());
    }

    void act () {
        if (core::get_key('A')) {
            impulse(Vec(-0.1, 0));
        }
        if (core::get_key('D')) {
            impulse(Vec(0.1, 0));
        }
    }
    void react () {
    }

    Rata () : Physical(bdf()), model(skel()) { }
    void start () { Physical::start(); Draws_Sprites::appear(); }
};

HCB_BEGIN(Rata)
    type_name("Rata");
    base<core::Stateful>("Rata");
    attr("physical", supertype<Physical>());
HCB_END(Rata)


